# Makefile for the contrib toollets
# Must be single file compile unit scriptlets please...

TOOLS_SOURCES:=cp2104-rs485.c
TOOLS:=$(TOOLS_SOURCES:.c=)

BIN=bin

LIST=$(addprefix $(BIN)/, $(TOOLS))

INCLUDES=-I../CP210xManufacturing -I../Common
LDFLAGS +=-L../Release/Linux
LDLIBS += -lcp210xmanufacturing

CFLAGS += -Wall -g -Os $(INCLUDES)
CXXFLAGS += -Wall -g -Os $(INCLUDES)

all: $(BIN) $(LIST)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/%:  %.cpp
	$(CXX) $(INCLUDES) $< $(CXXFLAGS) -o $@ $(LDLIBS) $(LDFLAGS)

#### Clean target deletes all generated files ####
clean:
	-$(RM) -r $(BIN)


