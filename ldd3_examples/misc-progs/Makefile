FILES = nbtest load50 mapcmp polltest mapper setconsole\
	datasize dataalign netifdebug

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
ARCHINCLUDE ?= $(KERNELDIR)/arch/x86/include
LINUXINCLUDE ?= $(KERNELDIR)/include
CFLAGS = -O2 -fomit-frame-pointer -Wall -I$(ARCHINCLUDE) \
	-I$(LINUXINCLUDE)

all: $(FILES)

clean:
	rm -f $(FILES) *~ core

