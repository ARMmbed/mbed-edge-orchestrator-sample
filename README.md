Simplified PT Client for mbed-edge

To compile (linux supported only):

	1) Clone and compile https://github.com/ARMmbed/mbed-edge-confidential-w (for simplicity, use developer-flow enrollment)

        2).Clone this repo

        3).Edit "build.sh" and adjust the EDGE_REPO variable to point to your compiled instance of "mbed-edge-confidential-w"

	4).Execute "./build.sh"

        5).In a separate window, launch the edge-core runtime

	6).Execute "./run.sh"

You should be able to confirm that there is a single device in your mbed Cloud account (it should be called "NonMbedDevice-0")

The shadowed device is created through PT (Protocol Translator). 

If interested in how the example works, start by examining "main.cpp" and carefully consider the "Orchestrator" class and its purpose. 

For sake of a simple example, this sample assumes a "simple" Non-mbed Device is to be shadowed via PT. 

The simple device simply "ticks" an updated counter value once every 25 seconds and also has a basic 1/0 switch state. 

The "tick" behavior of the device is modelled as an observable "counter" resource (URI: /123/0/4567) in mbed Cloud's "shadow" of the device

The 1/0 switch behavior of the device is modelled as a simple get/put switch resource (URI: /311/0/5850) in mbed Cloud's "shadow" of the device
