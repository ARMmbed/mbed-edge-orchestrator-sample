CXXFLAGS := $(CXXFLAGS) -g \
	-I. \
	-I./include \
	-I$(EDGE_REPO) \
	-I$(EDGE_REPO)/include \
	-I$(EDGE_REPO)/include/common \
	-I$(EDGE_REPO)/mbed-edge-module-sources/nanostack-libservice/mbed-client-libservice \
	-I$(EDGE_REPO)/lib/jsonrpc \
	-I$(EDGE_REPO)/mbed-edge-module-sources/pal/Source/PAL-Impl/Services-API \
	-I$(EDGE_REPO)/mbed-edge-module-sources/pal/Configs/pal_config \
	-I$(EDGE_REPO)/mbed-edge-module-sources/mbed-trace \
	-I$(EDGE_REPO)/pt-client \
	-D__LINUX__ -I$(EDGE_REPO)/mbed-edge-module-sources/pal/Configs/pal_config/Linux 

CFLAGS := $(CFLAGS) $(CXXFLAGS)

LIB_BASE := $(EDGE_REPO)/build/mcc-linux-x86

LIBS := $(LIB_BASE)/existing/pt-client/libpt-client.a \
	$(LIB_BASE)/existing/lib/libfstrm.a \
	$(LIB_BASE)/existing/edge-rpc/librpc.a \
	$(LIB_BASE)/existing/lib/libjsonrpc.a \
	$(LIB_BASE)/mbed-edge-modules/pal/existing/libpal.a \
	$(LIB_BASE)/mbed-edge-modules/mbed-client-esfs/source/libmbed-client-esfs.a \
	$(LIB_BASE)/mbed-edge-modules/pal/existing/libpal.a \
	$(LIB_BASE)/mbed-edge-modules/mbed-client-esfs/source/libmbed-client-esfs.a \
	$(LIB_BASE)/mbed-edge-modules/mbedtls/source/libmbedtls.a \
	$(LIB_BASE)/mbed-edge-modules/mbed-trace/source/libmbed-trace.a \
	$(LIB_BASE)/mbed-edge-modules/nanostack-libservice/source/libnanostack-libservice.a \
	-ljansson -levent -levent_pthreads -lrt -lpthread 

all: mbed-edge-orchestrator-sample.exe

mbed-edge-orchestrator-sample.exe: utils.o byte_order.o Orchestrator.o NonMbedDevice.o main.o DeviceShadow.o
	g++ -o mbed-edge-orchestrator-sample.exe Orchestrator.o NonMbedDevice.o main.o utils.o byte_order.o DeviceShadow.o $(LIBS)

clean:
	/bin/rm -f *.exe *.o core a.out
