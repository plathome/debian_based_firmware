SRCS = i2cBus.c 
_SRCS = pshd.c runled.c

OBJS = $(subst .c,.o,$(SRCS))
_OBJS = $(subst .c,.o,$(_SRCS))
__OBJS = $(OBJS) $(_OBJS)
dependencies = $(subst .o,.d,$(__OBJS))

all: pshd runled
.PHONY: all install clean

%d: %.c %.h
	$(CC) $(CFLAGS) -c $@.c	

pshd: $(OBJS) pshd.o
	$(CC) $(CFLAGS) -o $@ $@.o $(OBJS) $(LDFLAGS)

runled: $(OBJS) runled.o
	$(CC) $(CFLAGS) -o $@ $@.o $(OBJS) $(LDFLAGS)

clean:
	-rm -f pshd runled *.o *.core .gdbinit
