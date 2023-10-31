#!/bin/sh

default_env(){
	fw_setenv arch arm
	fw_setenv autoload no
	fw_setenv baudrate 1500000
	fw_setenv board evb_rk3568
	fw_setenv board_name evb_rk3568
	fw_setenv boot_a_script 'load ${devtype} ${devnum}:${distro_bootpart} ${scriptaddr} ${prefix}${script}; source ${scriptaddr}'
	fw_setenv boot_extlinux 'sysboot ${devtype} ${devnum}:${distro_bootpart} any ${scriptaddr} ${prefix}extlinux/extlinux.conf'
	fw_setenv boot_net_usb_start 'usb start'
	fw_setenv boot_prefixes '/ /boot/'
	fw_setenv boot_script_dhcp boot.scr.uimg
	fw_setenv boot_scripts 'boot.scr.uimg boot.scr'
	fw_setenv boot_targets 'mmc1 mmc0 mtd2 mtd1 mtd0 usb0 pxe dhcp' 
	fw_setenv bootargs 'storagemedia=emmc androidboot.storagemedia=emmc androidboot.mode=normal'
	fw_setenv bootcmd 'boot_android ${devtype} ${devnum};boot_fit;bootrkp;run distro_bootcmd;'
	fw_setenv bootcmd_dhcp 'run boot_net_usb_start; if dhcp ${scriptaddr} ${boot_script_dhcp}; then source ${scriptaddr}; fi;'
	fw_setenv bootcmd_mmc0 'setenv devnum 0; run mmc_boot'
	fw_setenv bootcmd_mmc1 'setenv devnum 1; run mmc_boot'
	fw_setenv bootcmd_mtd0 'setenv devnum 0; run mtd_boot'
	fw_setenv bootcmd_mtd1 'setenv devnum 1; run mtd_boot'
	fw_setenv bootcmd_mtd2 'setenv devnum 2; run mtd_boot'
	fw_setenv bootcmd_pxe 'run boot_net_usb_start; dhcp; if pxe get; then pxe boot; fi'
	fw_setenv bootcmd_usb0 'setenv devnum 0; run usb_boot'
	fw_setenv bootdelay 3
	fw_setenv cpu armv8
	fw_setenv devnum 0
	fw_setenv devtype mmc
	fw_setenv distro_bootcmd 'for target in ${boot_targets}; do run bootcmd_${target}; done'
	fw_setenv eth1addr ba:37:6d:cf:01:81
	fw_setenv ethaddr b6:37:6d:cf:01:81
	fw_setenv fdt_addr_r 0x08300000
	fw_setenv fdtcontroladdr 0x8300000
	fw_setenv kernel_addr_c 0x04080000
	fw_setenv kernel_addr_r 0x00280000
	fw_setenv mmc_boot 'if mmc dev ${devnum}; then setenv devtype mmc; run scan_dev_for_boot_part; fi'
	fw_setenv partitions 'uuid_disk=${uuid_gpt_disk};name=uboot,start=8MB,size=4MB,uuid=${uuid_gpt_loader2};name=trust,size=4M,uuid=${uuid_gpt_atf};name=misc,size=4MB,uuid=${uuid_gpt_misc};name=resource,size=16MB,uuid=${uuid_gpt_resource};name=kernel,size=32M,uuid=${uuid_gpt_kernel};name=boot,size=32M,bootable,uuid=${uuid_gpt_boot};name=recovery,size=32M,uuid=${uuid_gpt_recovery};name=backup,size=112M,uuid=${uuid_gpt_backup};name=cache,size=512M,uuid=${uuid_gpt_cache};name=system,size=2048M,uuid=${uuid_gpt_system};name=metadata,size=16M,uuid=${uuid_gpt_metadata};name=vendor,size=32M,uuid=${uuid_gpt_vendor};name=oem,size=32M,uuid=${uuid_gpt_oem};name=frp,size=512K,uuid=${uuid_gpt_frp};name=security,size=2M,uuid=${uuid_gpt_security};name=userdata,size=-,uuid=${uuid_gpt_userdata};'
	fw_setenv pxefile_addr_r 0x00e00000
	fw_setenv ramdisk_addr_r 0x0a200000
	fw_setenv rkimg_bootdev 'if mmc dev 1 && rkimgtest mmc 1; then setenv devtype mmc; setenv devnum 1; echo Boot from SDcard;elif mmc dev 0; then setenv devtype mmc; setenv devnum 0;elif mtd_blk dev 0; then setenv devtype mtd; setenv devnum 0;elif mtd_blk dev 1; then setenv devtype mtd; setenv devnum 1;elif mtd_blk dev 2; then setenv devtype mtd; setenv devnum 2;elif rknand dev 0; then setenv devtype rknand; setenv devnum 0;elif rksfc dev 0; then setenv devtype spinand; setenv devnum 0;elif rksfc dev 1; then setenv devtype spinor; setenv devnum 1;else;setenv devtype ramdisk; setenv devnum 0;fi;' 
	fw_setenv scan_dev_for_boot 'echo Scanning ${devtype} ${devnum}:${distro_bootpart}...; for prefix in ${boot_prefixes}; do run scan_dev_for_extlinux; run scan_dev_for_scripts; done;'
	fw_setenv scan_dev_for_boot_part 'part list ${devtype} ${devnum} -bootable devplist; env exists devplist || setenv devplist 1; for distro_bootpart in ${devplist}; do if fstype ${devtype} ${devnum}:${distro_bootpart} bootfstype; then run scan_dev_for_boot; fi; done'
	fw_setenv scan_dev_for_extlinux 'if test -e ${devtype} ${devnum}:${distro_bootpart} ${prefix}extlinux/extlinux.conf; then echo Found ${prefix}extlinux/extlinux.conf; run boot_extlinux; echo SCRIPT FAILED: continuing...; fi'
	fw_setenv scan_dev_for_scripts 'for script in ${boot_scripts}; do if test -e ${devtype} ${devnum}:${distro_bootpart} ${prefix}${script}; then echo Found U-Boot script ${prefix}${script}; run boot_a_script; echo SCRIPT FAILED: continuing...; fi; done'
	fw_setenv scriptaddr 0x00c00000
	fw_setenv serial# d071cbfeb3bc770a
	fw_setenv soc rockchip
	fw_setenv stderr serial,vidconsole
	fw_setenv stdin serial,usbkbd
	fw_setenv stdout serial,vidconsole
	fw_setenv temp_file_addr 0x08000000
	fw_setenv usb_boot 'usb start; if usb dev ${devnum}; then setenv devtype usb; run scan_dev_for_boot_part; fi'
	fw_setenv vendor rockchip
}

obsiot_env(){
	return
}

usage(){
	echo "Update uboot environment for OpenBlocks TB3N"
	echo
	echo "usage: $(basename $0) [-a]"
	echo
	echo "	-a	OpenBlocks TB3N default environment"
	echo "	-h	This messages"
}

if [ "$1" = "-a" ]; then
	obsiot_env
elif [ "$1" = "--rockchip-default" ]; then
	default_env
else
	usage
	exit 1
fi

exit 0
