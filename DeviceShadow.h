/**
 * @file    DeviceShadow.h
 * @brief   mbed Edge Devic Shadow Sample 
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

#ifndef __DEVICE_SHADOW_H__
#define __DEVICE_SHADOW_H__

// system includes
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

// mbed-edge PT includes
#include "common/constants.h"
#include "common/integer_length.h"

// Tunables for the shadow device
#define LIFETIME                    60			// 60 seconds before re-registration of the device
#define SAMPLE_DEVICE_PREFIX        "NonMbedDevice"	// Basically.. we are naming the device shadow "NonMbedDevice" per our "actual" device class underneath
#define SAMPLE_DEVICE_ENDPOINT_TYPE "mbed-endpoint"	// defaulted EPT (currently not implemented by PT)

// IPSO Object ID's used by our device shadow
enum IPSO_OBJECTS {
    COUNTER_OBJECT_ID = 123,	     // counter object ID
    SWITCH_OBJECT_ID  = 311	     // switch object ID
};

// IPSO Resource IDs used by our device shadow
enum IPSO_RESOURCES {
    COUNTER_RESOURCE_ID   = 4567,    // counter resource URI: /123/0/4567
    SWITCH_RESOURCE_ID    = 5850     // switch resource URI: /311/0/5850
};

// we have to wrap in "externs" since the headers dont have them already...
#ifdef __cplusplus
extern "C" {
#endif
    #include "pt-client/pt_api.h"
    #include "pt-client/pt_device_object.h"
#ifdef __cplusplus
};
#endif

class DeviceShadow {
    public:
	DeviceShadow(void *orchestrator);
	DeviceShadow(void *orchestrator,char *device_id,char *suffix);
        virtual ~DeviceShadow();

	// notify that the counter value has changed
	void notifyCounterValueHasChanged(int new_value);

	// process events
	void processEvents();

	// create and register the shadow device
	bool createAndRegister();

	// process a write request to the shadow device...
	bool processWriteRequest(const char *device_id, const uint16_t object_id, const uint16_t instance_id, const uint16_t resource_id, const unsigned int operation, const uint8_t *value, const uint32_t value_size);

        // write success
        void writeSuccess(const char *device_id);
        static void writeSuccessCB(const char *device_id,void *ctx);

        // write failure
        void writeFailure(const char *device_id);
        static void writeFailureCB(const char *device_id,void *ctx);

	// update counter value 
	void updateCounterValue(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size);
	static void updateCounterValueCB(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size,void *ctx);

        // update the switch state
	void updateSwitchState(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size);
        static void updateSwitchStateCB(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size,void *ctx);	

	// reboot callback
	void rebootDevice(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_length);
	static void rebootDeviceCB(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_length, void *ctx) ;

	// registration success
	void registrationSuccess(const char *device_id);
	static void registrationSuccessCB(const char* device_id, void *ctx);

	// registration failure
	void registrationFailure(const char *device_id);
	static void registrationFailureCB(const char* device_id, void *ctx);

	// update the counter resource value
	void updateCounterResourceValue(int value);

        // deregistration success
        void unregisterSuccess(const char *device_id);
        static void unregisterSuccessCB(const char* device_id, void *ctx);

        // deregistration failure
        void unregisterFailure(const char *device_id);
        static void unregisterFailureCB(const char* device_id, void *ctx);

	// deregister our device shadow from PT
	void deregister();

    private:
	DeviceShadow(const DeviceShadow &device);
        void initialize(void *orchestrator,char *device_id,char *suffix);
	pt_device_t *createPTDevice();
	pt_resource_opaque_t *getResourceInstance(const uint16_t object_id,const uint16_t instance_id,const uint16_t resource_id);
	bool createShadowWithPT();
	bool registerShadowWithPT();
	void createCounterLWM2MResource();
	void createSwitchLWM2MResource();
  	void *getDevice();

    private:
	void        *m_orchestrator;
        bool	     m_is_registered;
        pt_device_t *m_pt_device;
	char	    *m_device_id;
	char	    *m_suffix;
	char 	    *m_endpoint_id;

	int	     m_counter_value;
	bool	     m_switch_state;
	int	     m_new_counter_value;
	bool	     m_counter_value_changed;
};

#endif // __DEVICE_SHADOW_H__
