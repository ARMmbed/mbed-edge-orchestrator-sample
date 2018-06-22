/**
 * @file    DeviceShadow.cpp
 * @brief   mbed Edge DeviceShadow Implementation
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

#include "DeviceShadow.h"
#include "Orchestrator.h"
#include "NonMbedDevice.h"
#include "byte_order.h"

// constructor
DeviceShadow::DeviceShadow(void *orchestrator) {
    this->initialize(orchestrator,(char *)SAMPLE_DEVICE_PREFIX,(char *)"-0");
}

// constructor
DeviceShadow::DeviceShadow(void *orchestrator,char *device_id,char *suffix) {
    this->initialize(orchestrator,device_id,suffix);
}

// destructor
DeviceShadow::~DeviceShadow() {
   if (this->m_endpoint_id != NULL) {
	free(this->m_endpoint_id);
   }
   if (this->m_pt_device != NULL) {
	free(this->m_pt_device);
   }
}

// copy constructor
DeviceShadow::DeviceShadow(const DeviceShadow &device) {
}

// initialize
void DeviceShadow::initialize(void *orchestrator,char *device_id,char *suffix) {
    this->m_orchestrator = orchestrator;
    this->m_pt_device = NULL;
    this->m_device_id = device_id;
    this->m_suffix = suffix;
    this->m_is_registered = false;
    this->m_new_counter_value = -1;
    this->m_counter_value_changed = false;
    
    // create the FQ endpoint ID
    this->m_endpoint_id = (char *)malloc(strlen(this->m_device_id) + strlen(this->m_suffix) + 1);
    sprintf(this->m_endpoint_id, "%s%s", this->m_device_id, this->m_suffix);
}

// write success
void DeviceShadow::writeSuccess(const char *device_id) {
   printf("DeviceShadow: write SUCCESS for device %s\n",device_id);
}

// STATIC: write success CB
void DeviceShadow::writeSuccessCB(const char* device_id, void *ctx) {
   DeviceShadow *instance = (DeviceShadow *)ctx;
   if (instance != NULL) {
        instance->writeSuccess(device_id);
   }
}

// write failure 
void DeviceShadow::writeFailure(const char *device_id) {
   printf("DeviceShadow: write FAILURE for device %s\n",device_id);
}

// STATIC: write failure CB
void DeviceShadow::writeFailureCB(const char* device_id, void *ctx) {
   DeviceShadow *instance = (DeviceShadow *)ctx;
   if (instance != NULL) {
        instance->writeFailure(device_id);
   }
}

// create and register the shadow
bool DeviceShadow::createAndRegister() {
    if (this->createShadowWithPT() == true) {
	return this->registerShadowWithPT();
    }
    return false;
}

// create the device in PT
pt_device_t *DeviceShadow::createPTDevice() {
    pt_status_t status = PT_STATUS_SUCCESS;
    pt_device_t *device = pt_create_device(this->m_endpoint_id, LIFETIME, QUEUE, &status);
    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: ERROR. Could not create the device(%s) in PT...\n",this->m_endpoint_id);
        return NULL;
    }
    return device;
}

// update the counter value
void DeviceShadow::updateCounterValue(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size) {
    NonMbedDevice *device = (NonMbedDevice *)this->getDevice();
    long counter_value = 0;
    convert_value_to_host_order_long((uint8_t *)value,&counter_value);
    printf("DeviceShadow: Counter Value set to: %ld\n",counter_value);
    device->setCounterValue(counter_value);
}

// STATIC: update counter value
void DeviceShadow::updateCounterValueCB(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size, void *ctx) {   
    DeviceShadow *instance = (DeviceShadow *)ctx;
    if (instance != NULL) {
	instance->updateCounterValue(resource,value,value_size);
    }
}

// create the LWM2M Counter Resource
void DeviceShadow::createCounterLWM2MResource() {
    pt_status_t status = PT_STATUS_SUCCESS;
    NonMbedDevice *device = (NonMbedDevice *)this->getDevice();

    pt_object_t *object = pt_device_add_object(this->m_pt_device, COUNTER_OBJECT_ID, &status);
    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: Could not create an object with id (%d) to the device (%s).\n", COUNTER_OBJECT_ID, this->m_pt_device->device_id);
    }

    pt_object_instance_t *instance = pt_object_add_object_instance(object, 0, &status);
    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: Could not create an object instance with id: 0 to the object (%d).\n", COUNTER_OBJECT_ID);
    }

    uint8_t *counter_data = (uint8_t *)malloc(sizeof(int));
    convert_long_value_to_network_byte_order((long)device->getCounterValue(), (uint8_t *) counter_data);

    (void)pt_object_instance_add_resource_with_callback(instance, COUNTER_RESOURCE_ID,
                                          LWM2M_INTEGER,
                                          OPERATION_READ | OPERATION_WRITE,
                                          counter_data,
                                          sizeof(int),
                                          &status,
                                          &DeviceShadow::updateCounterValueCB);

    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: Could not create a resource with id (%d) to the object_instance 0.\n", COUNTER_RESOURCE_ID);
    }
}

// update the switch state 
void DeviceShadow::updateSwitchState(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size) {
    NonMbedDevice *device = (NonMbedDevice *)this->getDevice();
    long long_switch_state = 0;
    convert_value_to_host_order_long((uint8_t *)value,&long_switch_state);
    bool switch_state = false;
    if (long_switch_state != 0) {
	switch_state = true;
    }

    // set the switch state
    device->setSwitchState(switch_state);
}

// STATIC: update switch state
void DeviceShadow::updateSwitchStateCB(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_size, void *ctx) {
    DeviceShadow *instance = (DeviceShadow *)ctx;
    if (instance != NULL) {
        instance->updateSwitchState(resource,value,value_size);
    }
}

// create the LWM2M Switch Resource
void DeviceShadow::createSwitchLWM2MResource() {
    pt_status_t status = PT_STATUS_SUCCESS;
    NonMbedDevice *device = (NonMbedDevice *)this->getDevice();

    pt_object_t *object = pt_device_add_object(this->m_pt_device, SWITCH_OBJECT_ID, &status);
    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: Could not create an object with id (%d) to the device (%s).\n", SWITCH_OBJECT_ID, this->m_pt_device->device_id);
    }

    pt_object_instance_t *instance = pt_object_add_object_instance(object, 0, &status);
    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: Could not create an object instance with id 0 to the object (%d).\n", SWITCH_OBJECT_ID);
    }

    uint8_t *sw_data = (uint8_t *)malloc(sizeof(int));
    if (device->getSwitchState() == true) {
        convert_long_value_to_network_byte_order((long)1, (uint8_t *) sw_data);
    }
    else {
        convert_long_value_to_network_byte_order((long)0, (uint8_t *) sw_data);
    }

    (void)pt_object_instance_add_resource_with_callback(instance, SWITCH_RESOURCE_ID,
                                          LWM2M_INTEGER,
                                          OPERATION_READ | OPERATION_WRITE,
                                          sw_data,
                                          sizeof(int),
                                          &status,
                                          &DeviceShadow::updateSwitchStateCB);

    if (status != PT_STATUS_SUCCESS) {
        printf("DeviceShadow: Could not create a resource with id (%d) to the object_instance 0.\n", SWITCH_RESOURCE_ID);
    }
}

// reboot callback
void DeviceShadow::rebootDevice(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_length) {
    printf("DeviceShadow: Reboot device called.\n");
}

// STATIC: reboot callback
void DeviceShadow::rebootDeviceCB(const pt_resource_opaque_t *resource, const uint8_t* value, const uint32_t value_length, void *ctx) {
   DeviceShadow *instance = (DeviceShadow *)ctx;
   if (instance != NULL) {
	instance->rebootDevice(resource,value,value_length);
   }
}

// create shadow with PT
bool DeviceShadow::createShadowWithPT() {
    // create the device
    this->m_pt_device = this->createPTDevice();
    if (this->m_pt_device != NULL) { 
        // for our example, our shadow will contain a Counter LWM2M resource and a Switch LWM2M resource
        this->createCounterLWM2MResource();
        this->createSwitchLWM2MResource();
    
        // create a device object data
        ptdo_device_object_data_t *device_object_data = (ptdo_device_object_data_t *)malloc(sizeof(ptdo_device_object_data_t));
        device_object_data->manufacturer = strdup("ARM");
        device_object_data->model_number = strdup("1.0");
        device_object_data->serial_number = strdup("0123456789");
        device_object_data->firmware_version = strdup("N/A");
        device_object_data->hardware_version = strdup("N/A");
        device_object_data->software_version = strdup("N/A");
        device_object_data->device_type = strdup(SAMPLE_DEVICE_ENDPOINT_TYPE);
        device_object_data->reboot_callback = &DeviceShadow::rebootDeviceCB;
        device_object_data->factory_reset_callback = NULL;
        device_object_data->reset_error_code_callback = NULL;
        
        // now initialize the device
        ptdo_initialize_device_object(this->m_pt_device, device_object_data);
        
        // clean up
        free(device_object_data);
        
        // return our status
        return true;
    }
    return false;
}

// registration success
void DeviceShadow::registrationSuccess(const char *device_id) {
    printf("DeviceShadow: Shadow device: %s successfully registered\n",device_id);
    this->m_is_registered = true;

    // we are in the runPT thread within Orchestrator... we need to rejoin with the main thread...
    //printf("DeviceShadow: Rejoining with the main thread...\n");
    //pthread_exit(NULL);
}

// STATIC registration success CB
void DeviceShadow::registrationSuccessCB(const char* device_id, void *ctx) {
    DeviceShadow *instance = (DeviceShadow *)ctx;
    if (instance != NULL) {
	instance->registrationSuccess(device_id);
    }
}

// registration failure
void DeviceShadow::registrationFailure(const char *device_id) {
    printf("DeviceShadow: Shadow device: %s registration FAILED\n",device_id);
    this->m_is_registered = false;
}

// STATIC registration failure CB
void DeviceShadow::registrationFailureCB(const char* device_id, void *ctx) {
    DeviceShadow *instance = (DeviceShadow *)ctx;
    if (instance != NULL) {
        instance->registrationFailure(device_id);
    }
}

// register shadow with PT
bool DeviceShadow::registerShadowWithPT() {
    // DEBUG
    printf("ShadowDevice: Registering shadow device with mbed Cloud via PT...\n");
    Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;
    pt_register_device(orchestrator->getConnection(), this->m_pt_device, &DeviceShadow::registrationSuccessCB, &DeviceShadow::registrationFailureCB, (void *)this);
}

// find a specific resource instance
pt_resource_opaque_t *DeviceShadow::getResourceInstance(const uint16_t object_id,const uint16_t instance_id,const uint16_t resource_id) {
    pt_device_t *device = this->m_pt_device;
    pt_object_t *object = pt_device_find_object(device, object_id);
    pt_object_instance_t *instance = pt_object_find_object_instance(object, instance_id);
    return pt_object_instance_find_resource(instance, resource_id);
}

// process a write request
bool DeviceShadow::processWriteRequest(const char *device_id, const uint16_t object_id, const uint16_t instance_id, const uint16_t resource_id, const unsigned int operation, const uint8_t *value, const uint32_t value_size) {
    // DEBUG
    printf("DeviceShadow: processWriteRequest() URI: %s/%d/%d/%d value length: %d bytes\n",device_id,object_id,instance_id,resource_id,value_size);

    // get the approriate resource requested
    pt_resource_opaque_t *resource = this->getResourceInstance(object_id,instance_id,resource_id);
    if (resource == NULL) {
        printf("DeviceShadow: No match for device URI: %s/%d/%d/%d on write action.\n", device_id, object_id, instance_id, resource_id);
        return false;
    }

    /* Check if resource supports operation */
    if (!(resource->operations & operation)) {
        printf("DeviceShadow: Operation %d tried on resource URI: %s/%d/%d/%d which does not support it\n", operation, device_id, object_id, instance_id, resource_id);
        return false;
    }

    // for write and execute we typically have a callback... we may also update a value
    if (operation & OPERATION_WRITE || operation & OPERATION_EXECUTE) {
	// update the value in our resource (all of our resources are integers...)
	long new_value = 0;
	convert_value_to_host_order_long((const uint8_t *)value,&new_value);
	convert_long_value_to_network_byte_order(new_value,resource->value);

	// get the orchestrator
	Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;

        // DEBUG
	if (operation & OPERATION_WRITE) {
            printf("DeviceShadow: Writing new value URI: %s/%d/%d/%d value: %ld...\n", device_id, object_id, instance_id, resource_id,new_value);
	}
	if (operation & OPERATION_EXECUTE) {
	    printf("DeviceShadow: Executing new value URI: %s/%d/%d/%d value: %ld...\n", device_id, object_id, instance_id, resource_id,new_value);
	}

        // execute a callback if we have one...
        if (resource->callback != NULL) {
	    printf("DeviceShadow: Calling registered callback to write new resource value into mbed Cloud...\n");
            resource->callback(resource, value, value_size, this);
        }

        // update our value within PT if we have a value...
        if (value != NULL && value_size > 0) {
	    printf("DeviceShadow: Calling pt_write_value() to write new resource value into mbed Cloud...(thread id: %08x)\n",(unsigned int)pthread_self());
            pt_status_t status = pt_write_value(orchestrator->getConnection(), this->m_pt_device, this->m_pt_device->objects, &DeviceShadow::writeSuccessCB, &DeviceShadow::writeFailureCB, this);
	    if (status == PT_STATUS_SUCCESS) {
		// success
		printf("DeviceShadow: pt_write_value() succeeded!\n");
	    }
	    else {
		// failure
		printf("DeviceShadow: pt_write_value() failed with error: %d\n",status);
	    }
        }
    }
    return true;
}

// update the counter resource value via PT
void DeviceShadow::updateCounterResourceValue(int value) {
    // DEBUG
    printf("DeviceShadow:: updating device shadow counter resource to: %d (thread id: %08x)...\n",value,(unsigned int)pthread_self());
    pt_resource_opaque_t *resource = this->getResourceInstance(COUNTER_OBJECT_ID,0,COUNTER_RESOURCE_ID);
    Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;

    if (!resource) {
        printf("DeviceShadow: Could not find the device shadow counter object/resource.\n");
        return;
    }

    // get the current resource value
    long current = -1;
    convert_value_to_host_order_long(resource->value,&current);

    /* If value changed update it */
    if (current != (long)value) {
        current = (long)value; // current value is now the counter value incremented...
        printf("DeviceShadow: Updating counter value in mbed Cloud: %d\n",value);
        convert_long_value_to_network_byte_order(current,resource->value);
	if (resource->callback != NULL) {
            resource->callback(resource,resource->value,resource->value_size,this);
	}
	printf("DeviceShadow: Calling pt_write_value() to update counter resource in mbed Cloud: %d\n",value);
        pt_status_t status = pt_write_value(orchestrator->getConnection(), this->m_pt_device, this->m_pt_device->objects, &DeviceShadow::writeSuccessCB, &DeviceShadow::writeFailureCB,this);
	if (status == PT_STATUS_SUCCESS) {
           // success
           printf("DeviceShadow: pt_write_value() succeeded!\n");
        }
        else {
           // failure
           printf("DeviceShadow: pt_write_value() failed with error: %d\n",status);
        }
    }
}

// get our actual underlying device
void *DeviceShadow::getDevice() {
     Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;
     if (orchestrator != NULL) {
         return orchestrator->getDevice();
     }
     return NULL;
}

// unregistration success
void DeviceShadow::unregisterSuccess(const char *device_id) {
    printf("DeviceShadow: Shadow device: %s successfully deregistered\n",device_id);
    Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;
    orchestrator->completeShutdown();
}

// STATIC unregistration success CB
void DeviceShadow::unregisterSuccessCB(const char* device_id, void *ctx) {
    DeviceShadow *instance = (DeviceShadow *)ctx;
    if (instance != NULL) {
        instance->unregisterSuccess(device_id);
    }
}

// unregistration failure
void DeviceShadow::unregisterFailure(const char *device_id) {
    printf("DeviceShadow: Shadow device: %s deregistration FAILED\n",device_id);
    pt_device_free(this->m_pt_device);
    Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;
    orchestrator->completeShutdown();
}

// STATIC unregistration failure CB
void DeviceShadow::unregisterFailureCB(const char* device_id, void *ctx) {
    DeviceShadow *instance = (DeviceShadow *)ctx;
    if (instance != NULL) {
        instance->unregisterFailure(device_id);
    }
}

// deregister our shadow
void DeviceShadow::deregister() {
    printf("DeviceShadow: Unregistering device shadow from mbed Cloud via PT...\n");
    if (this->m_is_registered == true) {
        Orchestrator *orchestrator = (Orchestrator *)this->m_orchestrator;
        pt_status_t status = pt_unregister_device(orchestrator->getConnection(), this->m_pt_device, &DeviceShadow::unregisterSuccessCB, &DeviceShadow::unregisterFailureCB, this);
        if (PT_STATUS_SUCCESS != status) {
            pt_device_free(this->m_pt_device);
	}
    } 
}

// notify that the counter value has changed
void DeviceShadow::notifyCounterValueHasChanged(int new_value) {
    this->m_new_counter_value = new_value;
    this->m_counter_value_changed = true;
}

// process events
void DeviceShadow::processEvents() {
     printf("DeviceShadow: Checking to see if we have any events to process (thread id: %08x)...\n",(unsigned int)pthread_self());
     if (this->m_counter_value_changed == true) {
	printf("DeviceShadow: Counter has changed in the non-mbed device... updating the mapped resource in PT...\n");
	this->updateCounterResourceValue(this->m_new_counter_value);
        this->m_counter_value_changed = false;
	printf("DeviceShadow: Contuer value updated in resource via PT.\n");
     }
     else {
	// nothing to do
	printf("DeviceShadow: No events to process (OK)...\n");
     }
}
