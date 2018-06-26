/**
 * @file    Orchestrator.h
 * @brief   mbed Edge Orchestrator Sample
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

#ifndef __ORCHESTRATOR_H__
#define __ORCHESTRATOR_H__

// system includes
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

// mbed-edge PT includes
#include "common/constants.h"
#include "common/integer_length.h"

// we have to wrap in "externs" since the headers dont have them already...
#ifdef __cplusplus
extern "C" {
#endif
#include "pt-client/pt_api.h"
#include "pt-client/pt_device_object.h"
#ifdef __cplusplus
};
#endif

// DeviceShadow
#include "DeviceShadow.h"

// PT context
typedef struct protocol_translator_api_ctx {
  const char *hostname;
  int port;
  char *name;
} protocol_translator_api_ctx_t;

// PT Orchestrator
class Orchestrator {
public:
  Orchestrator(void *device);
  virtual ~Orchestrator();

  // static "tick" event handler processor
  static void tickHandler(int value, void *ctx);

  // process a "tick" event
  void processTick(int value);

  // main loop for Orchestrator (trivial sleep...)
  void processEvents();

  // connect the Orchestrator to mbed edge PT
  bool connectToMbedEdgePT(int argc, char **argv);

  // Run mbed edge's PT
  void runPT(void);
  static void *runPT(void *ctx);

  // PT Shutdown
  void completeShutdown();
  void shutdown();
  static void shutdownCB(void *ctx);

  // PT is Ready for processing!
  void ptIsReady(struct connection *connection);
  static void ptIsReadyCB(struct connection *connection, void *ctx);

  // Process device shadow write request
  static void processWriteRequestCB(struct connection *c, const char *device_id,
                                    const uint16_t object_id,
                                    const uint16_t instance_id,
                                    const uint16_t resource_id,
                                    const unsigned int operation,
                                    const uint8_t *value,
                                    const uint32_t value_size, void *ctx);

  // Get the device shadow (we only have one actual device and one shadow in our
  // example Orchestrator...)
  DeviceShadow *getDeviceShadow();

  // Get our actual underlying device
  void *getDevice();

  // Get our connection
  struct connection *getConnection();

  // PT Registation Success
  void ptRegisterSuccess();
  static void ptRegisterSuccessCB(void *ctx);

  // PT Registration Failure
  void ptRegisterFailure();
  static void ptRegisterFailureCB(void *ctx);

private:
  Orchestrator(const Orchestrator &orchestrator);
  void initialize(void *device);
  bool initializePT(int argc, char **argv);
  bool startPT();
  void createDeviceShadow(void);

private:
  // PT essentials
  bool m_pt_connected;
  struct connection *m_connection;
  protocol_translator_api_ctx_t *m_pt_ctx;
  pthread_t m_pt_thread;

  // device essentials - in our sample, we have ONE "actual" device and ONE
  // shadow representing it in mbed Cloud...
  void *m_device;                // the "actual" underlying device
  DeviceShadow *m_device_shadow; // the shadow of the "actual" device within
                                 // mbed Cloud (via PT)
};

#endif // __ORCHESTRATOR_H__
