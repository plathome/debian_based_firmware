###########################################
# Makefile for libcp210xmanufacturing.so
# using libusb-1.0
#
# Silicon Labs
# 10-22-2012
###########################################

OUT      ?= Release/Linux/

all: $(OUT)libcp210xmanufacturing.so.1.0 $(OUT)cp210xmanufacturing-example contrib

CC       ?= gcc
CFLAGS   ?= -Wall -fPIC

CXX      ?= g++
CXXFLAGS ?= -Wall -fPIC

COBJS     =
CPPOBJS   = CP210xManufacturing/CP210xDevice.o \
            CP210xManufacturing/CP2101Device.o \
            CP210xManufacturing/CP2102Device.o \
            CP210xManufacturing/CP2103Device.o \
            CP210xManufacturing/CP2104Device.o \
            CP210xManufacturing/CP2105Device.o \
            CP210xManufacturing/CP2108Device.o \
            CP210xManufacturing/CP2109Device.o \
            CP210xManufacturing/CP210xSupportFunctions.o \
	    CP210xManufacturing/CP210xManufacturing.o
OBJS      = $(COBJS) $(CPPOBJS)
LIBS      = `pkg-config libusb-1.0 --libs`
APP_LIBS = -L$(OUT) -lcp210xmanufacturing
INCLUDES ?= -I./Common -I./CP210xManufacturing `pkg-config libusb-1.0 --cflags`


$(OUT)libcp210xmanufacturing.so.1.0: $(OBJS)
	$(CXX) $(CXXFLAGS) -shared -Wl,-soname,libcp210xmanufacturing.so.1 -o $(OUT)libcp210xmanufacturing.so.1.0 $^ $(LIBS)
	ln -s libcp210xmanufacturing.so.1.0 $(OUT)libcp210xmanufacturing.so.1
	ln -s libcp210xmanufacturing.so.1.0 $(OUT)libcp210xmanufacturing.so

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

# Special case for demo app
main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

$(OUT)cp210xmanufacturing-example: main.o $(OUT)libcp210xmanufacturing.so.1.0
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< $(LIBS) $(APP_LIBS) -o $@

contrib: $(OUT)libcp210xmanufacturing.so.1.0
	$(MAKE) -C contrib

clean:
	$(RM) $(OBJS) $(OUT)libcp210xmanufacturing.so*
	$(RM) main.o $(OUT)cp210xmanufacturing-example
	$(RM) cp2104-rs485 cp2105-rs485
	$(MAKE) -C contrib clean

.PHONY: clean
