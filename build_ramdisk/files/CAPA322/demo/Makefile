#all: hwm comport test_com dio

#all: hwm wdt dio  
all: obshx1-wdt

# Compiler
CC = cc

# Include files
INCLUDE = .

# libs
LIB = axio

# Include libs
#INCLUDE_LIB = /usr/lib
INCLUDE_LIB = ../lib/64

# Options for development
CFLAGS = -Wall

obshx1-wdt:obshx1-wdt.c
	$(CC) $(CFLAGS) -o obshx1-wdt obshx1-wdt.c -l$(LIB) -L$(INCLUDE_LIB)

#wdt:wdt.c
#	$(CC) $(CFLAGS) -o wdt wdt.c -l$(LIB) -L$(INCLUDE_LIB)

#hwm: hwm.c
#	$(CC) $(CFLAGS) -o hwm hwm.c -l$(LIB) -L$(INCLUDE_LIB)
	
#dio: dio.c
#	$(CC) $(CFLAGS) -o dio dio.c -l$(LIB) -L$(INCLUDE_LIB)	

clean:
#	rm -f hwm  comport test_com dio  wdt
	rm -f obshx1-wdt
