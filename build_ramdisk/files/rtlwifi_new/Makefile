SHELL := /bin/sh
CC = gcc
KVER  ?= $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
FIRMWAREDIR := /lib/firmware/
PWD := $(shell pwd)
CLR_MODULE_FILES := *.mod.c *.mod *.o .*.cmd *.ko *~ .tmp_versions* modules.order Module.symvers
SYMBOL_FILE := Module.symvers
# Handle the move of the entire rtlwifi tree
ifneq ("","$(wildcard /lib/modules/$(KVER)/kernel/drivers/net/wireless/realtek)")
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/realtek/rtlwifi
else
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/rtlwifi
endif
#Handle the compression option for modules in 3.18+
ifneq ("","$(wildcard $(MODDESTDIR)/*.ko.gz)")
COMPRESS_GZIP := y
endif
ifneq ("","$(wildcard $(MODDESTDIR)/*.ko.xz)")
COMPRESS_XZ := y
endif

EXTRA_CFLAGS += -O2
obj-m := rtlwifi.o
rtlwifi-objs	:=	\
		base.o	\
		cam.o	\
		core.o	\
		debug.o	\
		efuse.o	\
		ps.o	\
		rc.o	\
		regd.o	\
		stats.o

obj-m	+= rtl_pci.o
rtl_pci-objs	:=		pci.o

obj-m	+= rtl_usb.o
rtl_usb-objs	:=		usb.o

obj-m	+= btcoexist/
obj-m	+= rtl8188ee/
obj-m	+= rtl8192c/
obj-m	+= rtl8192ce/
obj-m	+= rtl8192cu/
obj-m	+= rtl8192de/
obj-m	+= rtl8192ee/
obj-m	+= rtl8192se/
obj-m	+= rtl8723ae/
obj-m	+= rtl8723com/
obj-m	+= rtl8723be/
obj-m	+= rtl8821ae/

ccflags-y += -D__CHECK_ENDIAN__

all: 
	$(MAKE) -C $(KSRC) M=$(PWD) modules
install: all
ifeq (,$(wildcard ./backup_drivers.tar))
	@echo Making backups
	@tar cPf backup_drivers.tar $(MODDESTDIR)
endif

	@mkdir -p $(MODDESTDIR)/btcoexist
	@mkdir -p $(MODDESTDIR)/rtl8188ee
	@mkdir -p $(MODDESTDIR)/rtl8192c
	@mkdir -p $(MODDESTDIR)/rtl8192ce
	@mkdir -p $(MODDESTDIR)/rtl8192cu
	@mkdir -p $(MODDESTDIR)/rtl8192de
	@mkdir -p $(MODDESTDIR)/rtl8192ee
	@mkdir -p $(MODDESTDIR)/rtl8192se
	@mkdir -p $(MODDESTDIR)/rtl8723ae
	@mkdir -p $(MODDESTDIR)/rtl8723be
	@mkdir -p $(MODDESTDIR)/rtl8723com
	@mkdir -p $(MODDESTDIR)/rtl8821ae
	@install -p -D -m 644 rtl_pci.ko $(MODDESTDIR)	
	@install -p -D -m 644 rtl_usb.ko $(MODDESTDIR)	
	@install -p -D -m 644 rtlwifi.ko $(MODDESTDIR)
	@install -p -D -m 644 ./btcoexist/btcoexist.ko $(MODDESTDIR)/btcoexist
	@install -p -D -m 644 ./rtl8188ee/rtl8188ee.ko $(MODDESTDIR)/rtl8188ee
	@install -p -D -m 644 ./rtl8192c/rtl8192c-common.ko $(MODDESTDIR)/rtl8192c
	@install -p -D -m 644 ./rtl8192ce/rtl8192ce.ko $(MODDESTDIR)/rtl8192ce
	@install -p -D -m 644 ./rtl8192cu/rtl8192cu.ko $(MODDESTDIR)/rtl8192cu
	@install -p -D -m 644 ./rtl8192de/rtl8192de.ko $(MODDESTDIR)/rtl8192de
	@install -p -D -m 644 ./rtl8192ee/rtl8192ee.ko $(MODDESTDIR)/rtl8192ee
	@install -p -D -m 644 ./rtl8192se/rtl8192se.ko $(MODDESTDIR)/rtl8192se
	@install -p -D -m 644 ./rtl8723ae/rtl8723ae.ko $(MODDESTDIR)/rtl8723ae
	@install -p -D -m 644 ./rtl8723be/rtl8723be.ko $(MODDESTDIR)/rtl8723be
	@install -p -D -m 644 ./rtl8723com/rtl8723-common.ko $(MODDESTDIR)/rtl8723com
	@install -p -D -m 644 ./rtl8821ae/rtl8821ae.ko $(MODDESTDIR)/rtl8821ae
ifeq ($(COMPRESS_GZIP), y)
	@gzip -f $(MODDESTDIR)/*.ko
	@gzip -f $(MODDESTDIR)/btcoexist/*.ko
	@gzip -f $(MODDESTDIR)/rtl8*/*.ko
endif
ifeq ($(COMPRESS_XZ), y)
	@xz -f $(MODDESTDIR)/*.ko
	@xz -f $(MODDESTDIR)/btcoexist/*.ko
	@xz -f $(MODDESTDIR)/rtl8*/*.ko
endif

	@depmod -ae -b $(MOUNTDIR) -F $(MOUNTDIR)/boot/System.map $(KVER)

	@#copy firmware images to target folder
	@cp -fr firmware/rtlwifi/ $(FIRMWAREDIR)/
	@echo "Install rtlwifi SUCCESS"

uninstall:
ifneq (,$(wildcard ./backup_drivers.tar))
	@echo Restoring backups
	@tar xvPf backup_drivers.tar
endif
	
	@depmod -ae -b $(MOUNTDIR) -F $(MOUNTDIR)/boot/System.map $(KVER)
	
	@echo "Uninstall rtlwifi SUCCESS"

clean:
	@rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~ .*.o.d .cache.mk
	@rm -fr rtl8*/*.mod.c rtl8*/*.mod rtl8*/*.o rtl8*/.*.cmd rtl8*/*.ko rtl8*/*~ rtl8*/*.cmd rtl8*/.*.o.d
	@rm -fr bt*/*.mod.c bt*/*.mod bt*/*.o bt*/.*.cmd bt*/*.ko bt*/*~ bt*/*.cmd bt*/.*.o.d
	@rm -fr .tmp_versions
	@rm -fr Modules.symvers
	@rm -fr Module.symvers
	@rm -fr Module.markers
	@rm -fr modules.order rtl8*/modules.order bt*/modules.order
