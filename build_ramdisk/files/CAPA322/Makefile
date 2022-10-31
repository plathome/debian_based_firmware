K_VERSION:=$(shell uname -r | cut -c1-3 | sed 's/2\.[56]/2\.6/')

LBITS := $(shell getconf LONG_BIT)

ifeq ($(LBITS),64)
  lib_dir32_64=64
else
  lib_dir32_64=32
endif

LIB_PATH=/usr/lib/

all:
	cp ./lib/$(lib_dir32_64)/libaxio.so.1.0.0 $(LIB_PATH) ; \
	cd $(LIB_PATH) ; \
	ln -s libaxio.so.1.0.0 libaxio.so.0 ; \
	ln -s libaxio.so.0 libaxio.so

clean:
	cd $(LIB_PATH) ; \
	rm -f libaxio.*
