/**
 * @file    NonMbedDevice.h
 * @brief   Non-Mbed Simulated Device Sample 
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

#ifndef __NON_MBED_DEVICE_H__
#define __NON_MBED_DEVICE_H__

#include <stdio.h>
#include <pthread.h>

// Tunables
#define TICKER_SLEEP_TIME_SEC			25		// tick once every 25 seconds...

// tick event callback
typedef void (ticker_event_fn)(int value,void *ctx);

class NonMbedDevice {
	public:
		NonMbedDevice();
 		virtual ~NonMbedDevice();
		
		// set the "tick" event handler 
		void setEventCallbackHandler(ticker_event_fn *fn,void *ctx);

		// static "tick" processor 
		static void *tickerProcessor(void *ctx);
	
		// main loop for the simulated device (pthread)
		void start();
		void stop();
		void deviceRunLoop();

		// the simulated device "ticks" a counter value every "n" seconds... so we can get/set its value...
		void setCounterValue(int counter_value);
		int getCounterValue();

		// the simulated device has an I/O switch... so we can get/set its state...
	        void setSwitchState(bool switch_state);
		bool getSwitchState();

	private:
		NonMbedDevice(const NonMbedDevice &device);
		void initialize();
		void tick();

	private:
  		int m_counter;
		bool m_switch_state;

		ticker_event_fn *m_event_fn;
 		void *m_ctx;
  		bool m_is_running;
  	        pthread_t m_ticker_thread;
};

#endif // __NON_MBED_DEVICE_H__
