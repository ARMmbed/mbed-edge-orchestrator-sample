# Mbed Edge Orchestrator Sample

Simplified PT Client for mbed-edge

This PT sample creates the following simulated non-mbed device and "shadows" the device within mbed Cloud via mbed-edge:

![](`r https://github.com/ARMmbed/mbed-edge-orchestrator-sample/blob/master/description.png`)

## To compile (linux supported only):

	- Clone and compile https://github.com/ARMmbed/mbed-edge 
		- for simplicity, use developer-flow enrollment
	- Clone this repo
	- Edit "build.sh" and adjust the EDGE_REPO variable to point to your compiled instance of "mbed-edge"
	- Execute "./build.sh"
	- In a separate window, launch the edge-core runtime
	- Execute "./run.sh"

## Notes:

You should be able to confirm that there is a single device in your mbed Cloud account (it should be called "NonMbedDevice-0")

The shadowed device is created through PT (Protocol Translator). 

If interested in how the example works, start by examining "main.cpp" and carefully consider the "Orchestrator" class and its purpose. 

For sake of a simple example, this sample assumes a "simple" Non-mbed Device is to be shadowed via PT. 

The simple device simply "ticks" an updated counter value once every 25 seconds and also has a basic 1/0 switch state. 

The "tick" behavior of the device is modelled as an observable "counter" resource (URI: /123/0/4567) in mbed Cloud's "shadow" of the device

The 1/0 switch behavior of the device is modelled as a simple get/put switch resource (URI: /311/0/5850) in mbed Cloud's "shadow" of the device
