
CC = gcc
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/rtlwifi
FIRMWAREDIR := /lib/firmware/
PWD := $(shell pwd)
CLR_MODULE_FILES := *.mod.c *.mod *.o .*.cmd *.ko *~ .tmp_versions* modules.order Module.symvers
SYMBOL_FILE := Module.symvers

#Handle the compression option for modules in 3.18+
ifneq ("","$(wildcard $(MODDESTDIR)/*.ko.gz)")
COMPRESS_GZIP := y
endif
ifneq ("","$(wildcard $(MODDESTDIR)/*.ko.xz)")
COMPRESS_XZ := y
endif

EXTRA_CFLAGS += -O2
EXTRA_CFLAGS += -DDEBUG
obj-m := rtlwifi.o
PCI_MAIN_OBJS	:=	\
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

rtlwifi-objs += $(PCI_MAIN_OBJS)

ccflags-y += -D__CHECK_ENDIAN__

all: 
	$(MAKE) -C $(KSRC) M=$(PWD) modules
	@cp $(SYMBOL_FILE) btcoexist/
	+@make -C btcoexist/
	@cp $(SYMBOL_FILE) rtl8188ee/
	+@make -C rtl8188ee/
	@cp $(SYMBOL_FILE) rtl8192c/
	+@make -C rtl8192c/
	@cp $(SYMBOL_FILE) rtl8192ce/
	@cp rtl8192c/$(SYMBOL_FILE) rtl8192ce/
	+@make -C rtl8192ce/
	@cp $(SYMBOL_FILE) rtl8192cu/
	@cp rtl8192c/$(SYMBOL_FILE) rtl8192cu/
	+@make -C rtl8192cu/
	@cp $(SYMBOL_FILE) rtl8192de/
	+@make -C rtl8192de/
	@cp $(SYMBOL_FILE) rtl8192ee/
	@cp btcoexist/$(SYMBOL_FILE) rtl8192ee/
	+@make -C rtl8192ee/
	@cp $(SYMBOL_FILE) rtl8192se/
	+@make -C rtl8192se/
	@cp $(SYMBOL_FILE) rtl8723ae/
	@cp btcoexist/$(SYMBOL_FILE) rtl8723ae/
	+@make -C rtl8723ae/
	@cp $(SYMBOL_FILE) rtl8723be/
	@cp btcoexist/$(SYMBOL_FILE) rtl8723be/
	+@make -C rtl8723be/
	@cp $(SYMBOL_FILE) rtl8821ae/
	@cp btcoexist/$(SYMBOL_FILE) rtl8821ae/
	+@make -C rtl8821ae/
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
	@install -p -D -m 644 ./rtl8821ae/rtl8821ae.ko $(MODDESTDIR)/rtl8821ae
ifeq ($(COMPRESS_GZIP), y)
	@gzip -f $(MODDESTDIR)/*.ko
	@gzip -f $(MODDESTDIR)/btcoexist/*.ko
	@gzip -f $(MODDESTDIR)/rtl8*/*.ko
endif
ifeq ($(COMPRESS_XY), y)
	@xy -f $(MODDESTDIR)/*.ko
	@xy -f $(MODDESTDIR)/btcoexist/*.ko
	@xz -f $(MODDESTDIR)/rtl8*/*.ko
endif

	@depmod -a

	@#copy firmware images to target folder
	@cp -fr firmware/rtlwifi/ $(FIRMWAREDIR)/
	@echo "Install rtlwifi SUCCESS"

uninstall:
ifneq (,$(wildcard ./backup_drivers.tar))
	@echo Restoring backups
	@tar xvPf backup_drivers.tar
endif
	
	@depmod -a
	
	@echo "Uninstall rtlwifi SUCCESS"

clean:
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
	rm -fr Modules.symvers
	rm -fr Module.symvers
	rm -fr Module.markers
	rm -fr modules.order
	rm -fr tags
	@find -name "tags" -exec rm {} \;
	@rm -fr $(CLR_MODULE_FILES)
	@make -C btcoexist/ clean
	@make -C rtl8192c/ clean
	@make -C rtl8192ce/ clean
	@make -C rtl8192se/ clean
	@make -C rtl8192de/ clean
	@make -C rtl8723ae/ clean
	@make -C rtl8188ee/ clean
	@make -C rtl8723be/ clean
	@make -C rtl8192ee/ clean
	@make -C rtl8821ae/ clean
