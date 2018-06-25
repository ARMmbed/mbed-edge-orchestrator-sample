/**
 * @file    NonMbedDevice.cpp
 * @brief   Non Mbed Simulated Device Implemewntation
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

#include "NonMbedDevice.h"
#include <unistd.h>

// constructor
NonMbedDevice::NonMbedDevice() {
     this->initialize();
}

// destructor
NonMbedDevice::~NonMbedDevice() {
}

// copy constructor
NonMbedDevice::NonMbedDevice(const NonMbedDevice &device) {
}

// initialize the device
void NonMbedDevice::initialize() {
    this->m_event_fn = NULL;
    this->m_switch_state = false;
    this->m_counter = 0;
    this->m_ctx = NULL;
    this->m_is_running = true;
}

// set the event callback handler
void NonMbedDevice::setEventCallbackHandler(ticker_event_fn *fn, void *ctx) {
     this->m_event_fn = fn;
     this->m_ctx = ctx;
}

// STATIC: pthread invocation function
void *NonMbedDevice::tickerProcessor(void *ctx) {
    NonMbedDevice *instance = (NonMbedDevice *)ctx;
    if (instance != NULL) {
	instance->deviceRunLoop();
    }
    else {
	printf("NonMbedDevice: ERROR: NULL instance poniter... unable to run ticker processor..\n");
    }
    return NULL;
}

// device run loop
void NonMbedDevice::deviceRunLoop() {
     // loop
     printf("NonMbedDevice: non mbed device loop starting...(thread id: %08x)...\n",(unsigned int)pthread_self());
     while(this->m_is_running == true) {
        this->tick();
        sleep(TICKER_SLEEP_TIME_SEC);
     }

}

// start the device event loop
void NonMbedDevice::start() {
     // DEBUG
     printf("NonMbedDevice::starting the device event loop...\n");

     // create a simple thread to start a monotonic counter...
     pthread_create(&this->m_ticker_thread,NULL,&NonMbedDevice::tickerProcessor,(void *)this);
}

// stop the device event loop
void NonMbedDevice::stop() {
    // DEBUG
    printf("NonMbedDevice: Stopping device event loop...\n");
    this->m_is_running = false;
}

// set the switch state
void NonMbedDevice::setSwitchState(bool switch_state) {
    this->m_switch_state = switch_state;
    
    // DEBUG 
    printf("NonMbedDevice: Switch State set: %s\n", this->m_switch_state ? "on" : "off");
}

// get the switch state
bool NonMbedDevice::getSwitchState() {
     return this->m_switch_state;
}

// get the counter value
int NonMbedDevice::getCounterValue() {
    return this->m_counter;
}

// tick
void NonMbedDevice::tick() {
    // increment our counter
    ++this->m_counter;

    // DEBUG
    printf("NonMbedDevice: TICK(%d)...\n",this->m_counter);

    // call handler if we have one
    if (this->m_event_fn != NULL) {
	(this->m_event_fn)(this->m_counter,this->m_ctx);
    }
}

// (re)set our counter value
void NonMbedDevice::setCounterValue(int counter_value) {
    this->m_counter = counter_value;

    // DEBUG 
    printf("NonMbedDevice: Counter value set to: %d\n", this->m_counter);
}
