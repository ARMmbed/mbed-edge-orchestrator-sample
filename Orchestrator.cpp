/**
 * @file    Orchestrator.cpp
 * @brief   mbed Edge Orchestrator Implementation
 * @author  Doug Anson
 * @version 1.0
 * @see
 *
 * Copyright (c) 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Orchestrator.h"

// simulated device
#include "NonMbedDevice.h"

// Docooptargs support
#include "docoptargs.h"

// program ender
extern "C" void end_program();

// default constructor
Orchestrator::Orchestrator(void *device) { this->initialize(device); }

// destructor
Orchestrator::~Orchestrator() {
  if (this->m_connection != NULL) {
    free(this->m_connection);
  }
  if (this->m_device_shadow != NULL) {
    delete this->m_device_shadow;
  }
  if (this->m_pt_ctx != NULL) {
    free(this->m_pt_ctx);
  }
}

// copy constructor
Orchestrator::Orchestrator(const Orchestrator &orchestrator) {}

// initialize the orchestrator
void Orchestrator::initialize(void *device) {
  // bind to the actual underlying device and init...
  this->m_device = device;
  this->m_connection = NULL;
  this->m_pt_connected = false;

  // create our device shadow (only one for the one actual device we have
  // underneath...)
  this->m_device_shadow = new DeviceShadow((void *)this);
}

// get the connection
struct connection *Orchestrator::getConnection() {
  return this->m_connection;
}

// initialize PT
bool Orchestrator::initializePT(int argc, char **argv) {
  if (this->m_pt_ctx == NULL) {
    // initialize PT
    pt_client_initialize_trace_api();

    // parse args
    DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "0.1");

    // allocate and configure the protocol translator
    this->m_pt_ctx = (protocol_translator_api_ctx_t *)malloc(
        sizeof(protocol_translator_api_ctx_t));
    if (!args.protocol_translator_name) {
      printf("Missing required options: --protocol-translator-name parameter "
             "is mandatory\n");
      return false;
    }
    this->m_pt_ctx->name = strdup(args.protocol_translator_name);
    if (!args.port) {
      printf("Missing required options: --port parameter is mandatory\n");
      return false;
    }
    this->m_pt_ctx->port = atoi(args.port);
    this->m_pt_ctx->hostname = strdup(args.host);
  }
  return true;
}

// complete shutdown
void Orchestrator::completeShutdown() {
  // close our connection to PT...
  if (this->m_connection != NULL) {
    pt_client_shutdown(this->m_connection);
  }

  // end our program
  end_program();
}

// shutdown
void Orchestrator::shutdown() {
  // DEBUG
  printf("Orchestrator: Shutting down...\n");

  // stop our NonMbedDevice event loop
  if (this->m_device != NULL) {
    NonMbedDevice *d = (NonMbedDevice *)this->m_device;
    d->stop();
  }

  // deregister all shadows
  if (this->m_device_shadow != NULL) {
    this->m_device_shadow->deregister();
  }
}

// STATIC shutdown
void Orchestrator::shutdownCB(void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    instance->shutdown();
  }
}

// PT Registration Failure
void Orchestrator::ptRegisterFailure() { this->shutdown(); }

// STATIC: PT Registration failure
void Orchestrator::ptRegisterFailureCB(void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    instance->ptRegisterFailure();
  }
}

// PT Registration success
void Orchestrator::ptRegisterSuccess() {
  // DEBUG
  printf("Orchestrator: PT connected and registered. Ready for device shadow "
         "creation...\n");

  // we now have a connected and registered PT!
  this->m_pt_connected = true;

  // so, next wecreate the "shadow" of our device through PT...
  this->createDeviceShadow();
}

// STATIC: PT Registration success
void Orchestrator::ptRegisterSuccessCB(void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    instance->ptRegisterSuccess();
  }
}

// PT Connection is Ready
void Orchestrator::ptIsReady(struct connection *connection) {
  pt_status_t status = pt_register_protocol_translator(
      connection, &Orchestrator::ptRegisterSuccessCB,
      &Orchestrator::ptRegisterFailureCB, (void *)this);
  if (status != PT_STATUS_SUCCESS) {
    this->shutdown();
  }
}

// STATIC: process a write request
void Orchestrator::processWriteRequestCB(
    struct connection *c, const char *device_id, const uint16_t object_id,
    const uint16_t instance_id, const uint16_t resource_id,
    const unsigned int operation, const uint8_t *value,
    const uint32_t value_size, void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    // we simply have one shadow in our orchestrator.. so snag it and call its
    // processWriteRequest() method...
    bool success = instance->getDeviceShadow()->processWriteRequest(
        device_id, object_id, instance_id, resource_id, operation, value,
        value_size);
    if (success == true) {
      // write succees
      printf("Orchestrator: write SUCCESS\n");
    } else {
      // write failure
      printf("Orchestrator: write FAILURE\n");
    }
  }
}

// get the device shadow
DeviceShadow *Orchestrator::getDeviceShadow() { return this->m_device_shadow; }

// get the underlying device
void *Orchestrator::getDevice() { return this->m_device; }

// STATIC: PT Connection is Ready CB
void Orchestrator::ptIsReadyCB(struct connection *connection, void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    instance->ptIsReady(connection);
  }
}

// Run PT
void Orchestrator::runPT() {
  // DEBUG
  printf(
      "Orchestrator: starting up the protocol translator (ThreadID: %08x)...\n",
      (unsigned int)pthread_self());

  // create and run the protocol translator (PT) - configure the callbacks for
  // it...
  protocol_translator_callbacks_t pt_cbs;
  pt_cbs.connection_ready_cb =
      (pt_connection_ready_cb)&Orchestrator::ptIsReadyCB;
  pt_cbs.received_write_cb =
      (pt_received_write_handler)&Orchestrator::processWriteRequestCB;
  pt_cbs.connection_shutdown_cb =
      (pt_connection_shutdown_cb)&Orchestrator::shutdownCB;

  // start the PT...
  pt_client_start(this->m_pt_ctx->hostname, this->m_pt_ctx->port,
                  this->m_pt_ctx->name, &pt_cbs, (void *)this,
                  &this->m_connection);
}

// STATIC: Run PT
void *Orchestrator::runPT(void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    instance->runPT();
  }
  return NULL;
}

// Start PT
bool Orchestrator::startPT() {
  // create a thread to create PT, register it, create and register the device
  // shadow...
  pthread_create(&this->m_pt_thread, NULL, &Orchestrator::runPT, (void *)this);
}

// connect to mbed edge via PT
bool Orchestrator::connectToMbedEdgePT(int argc, char **argv) {
  // DEBUG
  printf("Orchestrator: connecting to mbed-edge via PT...\n");

  // initialize PT
  if (this->initializePT(argc, argv) == true) {
    // start PT
    this->startPT();

    // return current status
    return true;
  }

  // return our connection status
  return false;
}

// main event loop for the orchestrator
void Orchestrator::processEvents() {
  // the orchestrator can do other things in an actual implementation.. here we
  // can just sleep as our NonMbedDevice has an event loop and will drive
  // eventing via "ticks"
  while (true) {
    // process any events in the device shadow
    this->m_device_shadow->processEvents();

    // DEBUG
    printf("Orchestrator: Device shadow done processing events... sleeping for "
           "a bit...\n");

    // wait a bit
    sleep(5);
  };
}

// create our device shadow
void Orchestrator::createDeviceShadow() {
  // make sure that PT is connected and ready...
  if (this->m_pt_connected == true) {
    // have the device shadow create and register itself via PT
    this->m_device_shadow->createAndRegister();
  }
}

// device shadow: tick processor (ORCHESTRATE!)
void Orchestrator::processTick(int value) {
  // make sure that PT is connected and ready...
  if (this->m_pt_connected == true) {
    // DEBUG
    printf("Orchestrator: notifying device shadow that the counter value has "
           "changed. new_value=%d...\n",
           value);

    // tell the device shadow that the counter value has changed... its running
    // in a separate thread and will hand this in its loop...
    this->m_device_shadow->notifyCounterValueHasChanged(value);
  }
}

// STATIC: devide shadow: tick processor handler
void Orchestrator::tickHandler(int value, void *ctx) {
  Orchestrator *instance = (Orchestrator *)ctx;
  if (instance != NULL) {
    instance->processTick(value);
  } else {
    // null instance
    printf("Orchestrator: NULL instance, unable to process tick(%d)...\n",
           value);
  }
}
