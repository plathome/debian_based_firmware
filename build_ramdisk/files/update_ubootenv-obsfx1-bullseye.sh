#!/bin/sh

default_env(){
	fw_setenv baudrate 115200
	fw_setenv board_name EVK
	fw_setenv board_rev iMX8MP
	fw_setenv boot_fit no
	fw_setenv bootcmd 'mmc dev ${mmcdev}; if mmc rescan; then if run loadbootscript; then run bootscript; else if run loadimage; then run mmcboot; else run netboot; fi; fi; fi;'
	fw_setenv bootcmd_mfg 'run mfgtool_args;if iminfo ${initrd_addr}; then if test ${tee} = yes; then bootm ${tee_addr} ${initrd_addr} ${fdt_addr}; else booti ${loadaddr} ${initrd_addr} ${fdt_addr}; fi; else echo "Run fastboot ..."; fastboot 0; fi;'
	fw_setenv bootdelay 2
	fw_setenv bootscript 'echo Running bootscript from mmc ...; source'
	fw_setenv console ttymxc1,115200
	fw_setenv emmc_dev 2
	fw_setenv ethprime eth1
	fw_setenv fastboot_dev mmc1
	fw_setenv fdt_addr 0x43000000
	fw_setenv fdt_file imx8mp-evk.dtb
	fw_setenv fdt_high 0xffffffffffffffff
	fw_setenv fdtcontroladdr bcbf73b8
	fw_setenv image Image
	fw_setenv initrd_addr 0x43800000
	fw_setenv initrd_high 0xffffffffffffffff
	fw_setenv jh_mmcboot 'setenv fdt_file imx8mp-evk-root.dtb;setenv jh_clk clk_ignore_unused; if run loadimage; then run mmcboot; else run jh_netboot; fi;'
	fw_setenv jh_netboot 'setenv fdt_file imx8mp-evk-root.dtb; setenv jh_clk clk_ignore_unused; run netboot;'
	fw_setenv kboot booti 
	fw_setenv loadaddr 0x40480000
	fw_setenv loadbootscript 'fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};'
	fw_setenv loadfdt 'fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}'
	fw_setenv loadimage 'fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}'
	fw_setenv mfgtool_args 'setenv bootargs console=${console},${baudrate} rdinit=/linuxrc clk_ignore_unused'
	fw_setenv mmcargs 'setenv bootargs ${jh_clk} console=${console} root=${mmcroot}'
	fw_setenv mmcautodetect yes
	fw_setenv mmcboot 'echo Booting from mmc ...; run mmcargs; if test ${boot_fit} = yes || test ${boot_fit} = try; then bootm ${loadaddr}; else if run loadfdt; then booti ${loadaddr} - ${fdt_addr}; else echo WARN: Cannot load the DT; fi; fi;'
	fw_setenv mmcdev 1
	fw_setenv mmcpart 1
	fw_setenv mmcroot /dev/mmcblk1p2 rootwait rw
	fw_setenv nandfit_part yes
	fw_setenv netargs 'setenv bootargs ${jh_clk} console=${console} root=/dev/nfs ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp'
	fw_setenv netboot 'echo Booting from net ...; run netargs;  if test ${ip_dyn} = yes; then setenv get_cmd dhcp; else setenv get_cmd tftp; fi; ${get_cmd} ${loadaddr} ${image}; if test ${boot_fit} = yes || test ${boot_fit} = try; then bootm ${loadaddr}; else if ${get_cmd} ${fdt_addr} ${fdt_file}; then booti ${loadaddr} - ${fdt_addr}; else echo WARN: Cannot load the DT; fi; fi;'
	fw_setenv script boot.scr
	fw_setenv sd_dev 1
	fw_setenv serial# 140a7800dadb8cfc
	fw_setenv soc_type imx8mp
	fw_setenv splashimage 0x50000000
# remove add env
	fw_setenv fdt_usb_boot_file
	fw_setenv chkinit
	fw_setenv loadusb
	fw_setenv usbboot
	fw_setenv bootdev
	fw_setenv ethaddr
	fw_setenv eth1addr
	fw_setenv ethact
	fw_setenv ethprime
}

obsiot_env(){
	fw_setenv env_version 'v01'
	fw_setenv miscargs
	fw_setenv fdt_file imx8mp-evk-obsfx1-vsc8531.dtb
	fw_setenv fdt_usb_boot_file imx8mp-evk-obsfx1-vsc8531-usb-boot.dtb
	fw_setenv chkinit 'setenv noflashcfg; gpio input gpio3_22;if test ${$?} = 0; then setenv noflashcfg noflashcfg=1; fi;'
	fw_setenv mmcargs 'setenv bootargs ${jh_clk} console=${console} root=${mmcroot} ${noflashcfg} ${miscargs}'
	fw_setenv mmcboot 'echo Booting from mmc ...; run chkinit; run mmcargs; if test ${boot_fit} = yes || test ${boot_fit} = try; then bootm ${loadaddr}; else if run loadfdt; then booti ${loadaddr} - ${fdt_addr}; else echo WARN: Cannot load the DT; fi; fi;'
	fw_setenv loadusb 'ext4load usb 0:1 ${loadaddr} /boot/${image}; ext4load usb 0:1 ${fdt_addr} /boot/${fdt_usb_boot_file}'
	fw_setenv usbboot 'usb start; run loadusb; setenv mmcroot /dev/sda1; run chkinit; setenv miscargs ${miscargs} rootdelay=10; run mmcargs; booti ${loadaddr} - ${fdt_addr}'
	fw_setenv nvmeboot 'echo Booting from NVMe ...; run chkinit; setenv mmcroot /dev/nvme0n1p1; run mmcargs; run loadimage; run loadfdt; booti ${loadaddr} - ${fdt_addr};'
	fw_setenv rdboot 'echo Booting from Ramdisk ...; run chkinit; setenv mmcroot /dev/ram; run mmcargs; run loadimage; bootm ${loadaddr};'
	fw_setenv bootcmd 'mmc dev ${mmcdev}; if mmc rescan; then if run loadbootscript; then run bootscript; else if run loadimage; then run ${bootdev}; else run netboot; fi; fi; fi;'
	[ ! `fw_printenv | grep "^bootdev" 2> /dev/null` ] && fw_setenv bootdev mmcboot
}

usage(){
	echo "Update uboot environment for OpenBlocks IoT FX1"
	echo
	echo "usage: $(basename $0) [-a]"
	echo
	echo "	-a			OpenBlocks IoT FX1 default environment"
	echo "	--imx8mp-default	imx8mp evk board default environment"
	echo "	-h	This messages"
}

if [ "$1" = "-a" ]; then
	obsiot_env
elif [ "$1" = "--imx8mp-default" ]; then
#	dd if=/dev/zero of=/dev/mmcblk2 bs=1K seek=4096 count=4
	default_env
else
	usage
	exit 1
fi

exit 0
