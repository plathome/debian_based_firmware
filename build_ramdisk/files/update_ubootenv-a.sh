#!/bin/sh

default_env(){
	fw_setenv boot_target_cmd 'run do_flash_os;run do_probe_dfu;run do_compute_target;run mmc-bootargs;run load_kernel;zboot \${loadaddr}'
	fw_setenv bootargs_console 'console=ttyMFD2 earlyprintk=ttyMFD2,keep'
	fw_setenv bootargs_debug 'loglevel=4'
	fw_setenv bootargs_target 'multi-user'
	fw_setenv bootcmd 'echo "Target:\${target_name}"; run do_partition; run do_handle_bootargs_mode;'
	fw_setenv bootdelay '1'
	fw_setenv dfu_alt_info_ram 'kernel ram \${loadaddr} 0x800000'
	fw_setenv dfu_alt_info_reset 'reset ram 0x0 0x0'
	fw_setenv dfu_to_sec '3'
	fw_setenv do_boot 'run boot_target_cmd;'
	fw_setenv do_compute_target 'if itest.b \${first_install_retry} -gt \${first_install_max_retries} || itest.b \${ota_update_retry} -gt \${ota_update_max_retries}; then echo "Switch to Rescue target"; setenv bootargs_target rescue; saveenv; fi'
	fw_setenv do_dfu_alt_info_ifwi 'setenv dfu_alt_info "ifwi\${hardware_id} mmc 0 8192 mmcpart 1;ifwib\${hardware_id} mmc 0 8192 mmcpart 2"'
	fw_setenv do_dfu_alt_info_mmc 'setenv dfu_alt_info "ifwi\${hardware_id} mmc 0 8192 mmcpart 1;ifwib\${hardware_id} mmc 0 8192 mmcpart 2;u-boot0 part 0 1;u-boot-env0 part 0 2;u-boot1 part 0 3;u-boot-env1 part 0 4;boot part 0 7;rootfs part 0 8;update part 0 9;home part 0 10;vmlinuz fat 0 7;initrd fat 0 7"'
	fw_setenv do_dnx 'setenv dfu_alt_info \${dfu_alt_info_ram};dfu 0 ram 0 ram;run bootcmd'
	fw_setenv do_fallback 'echo "Unknown boot mode: \$bootargs_mode"; env delete -f bootargs_mode; saveenv; echo "Resetting to default boot mode and reboot..."; reset;'
	fw_setenv do_flash 'run do_force_flash_os;'
	fw_setenv do_flash_ifwi 'run do_dfu_alt_info_ifwi ; dfu 0 mmc 0'
	fw_setenv do_flash_os 'if itest.b \${do_flash_os_done} -eq 1 ; then echo "Flashing already done..." ; else run do_force_flash_os; fi'
	fw_setenv do_flash_os_done '1'
	fw_setenv do_flashall 'run do_partition;run do_flash_ifwi;run do_flash_os'
	fw_setenv do_force_flash_os 'run do_dfu_alt_info_mmc ; sleep 1 ; setenv do_flash_os_done 1 ; saveenv ; dfu 0 mmc 0'
	fw_setenv do_force_partition 'echo "Partitioning using GPT"; gpt write mmc 0 \${partitions} ; mmc rescan; setenv do_partition_done 1 ; saveenv'
	fw_setenv do_handle_bootargs_mode 'run do_preprocess_bootargs_mode; if itest.s \$bootargs_mode == "ota" ; then run do_ota; fi; if itest.s \$bootargs_mode == "boot" ; then run do_boot; fi; if itest.s \$bootargs_mode == "flash"; then run do_flash; fi; run do_fallback; exit;'
	fw_setenv do_load_ota_scr 'if fatload mmc 0:9 \$ota_script_addr ota_update.scr ; then setenv ota_status 0 ; else setenv ota_status 1 ; fi'
	fw_setenv do_ota 'run do_ota_init ; run do_load_ota_scr ; run do_source_ota_scr ; run do_ota_clean'
	fw_setenv do_ota_clean 'saveenv ; reset'
	fw_setenv do_ota_init 'setenv ota_status 1 ; env delete -f bootargs_mode'
	fw_setenv do_partition 'if itest.b \${do_partition_done} -eq 1; then echo "Partitioning already done..."; else run do_force_partition ; fi'
	fw_setenv do_partition_done '1'
	fw_setenv do_preprocess_bootargs_mode 'if env exists bootargs_mode ; then ; else setenv bootargs_mode "boot" ;fi;'
	fw_setenv do_probe_dfu 'run do_dfu_alt_info_mmc ; dfu 0 mmc 0 \$dfu_to_sec'
	fw_setenv do_source_ota_scr 'if test \$ota_status -eq 0 ; then if source \$ota_script_addr ; then setenv ota_status 0 ; else setenv ota_status 2 ; fi ; fi'
	fw_setenv first_install_max_retries '3'
	fw_setenv first_install_retry '0'
	fw_setenv hardware_id '00'
	fw_setenv init_dfu 'run do_dfu_alt_info_mmc ; saveenv'
	fw_setenv load_kernel 'fatload mmc 0:7 \${loadaddr} vmlinuz'
	fw_setenv loadaddr '0x100000'
	fw_setenv mmc-bootargs 'setenv bootargs root=PARTUUID=\${uuid_rootfs} rootfstype=ext4 \${bootargs_console} \${bootargs_debug} systemd.unit=\${bootargs_target}.target hardware_id=\${hardware_id} g_multi.iSerialNumber=\${serial#} g_multi.dev_addr=\${usb0addr}'
	fw_setenv ota_script_addr '0x100000'
	fw_setenv ota_update_max_retries '3'
	fw_setenv ota_update_retry '0'
	fw_setenv partitions 'uuid_disk=\${uuid_disk};name=u-boot0,start=1MiB,size=2MiB,uuid=\${uuid_uboot0};name=u-boot-env0,size=1MiB,uuid=\${uuid_uboot_env0};name=u-boot1,size=2MiB,uuid=\${uuid_uboot1};name=u-boot-env1,size=1MiB,uuid=\${uuid_uboot_env1};name=factory,size=1MiB,uuid=\${uuid_factory};name=panic,size=24MiB,uuid=\${uuid_panic};name=boot,size=32MiB,uuid=\${uuid_boot};name=rootfs,size=512MiB,uuid=\${uuid_rootfs};name=update,size=768MiB,uuid=\${uuid_update};name=home,size=-,uuid=\${uuid_home};'
	fw_setenv serial# 'f1cc7e1951f804f942a18039debc8709'
	fw_setenv stderr 'serial'
	fw_setenv stdin 'serial'
	fw_setenv stdout 'serial'
	fw_setenv target_name 'blank'
	fw_setenv usb0addr '02:00:86:bc:87:09'
	fw_setenv uuid_boot 'db88503d-34a5-3e41-836d-c757cb682814'
	fw_setenv uuid_disk '21200400-0804-0146-9dcc-a8c51255994f'
	fw_setenv uuid_factory '333a128e-d3e3-b94d-92f4-d3ebd9b3224f'
	fw_setenv uuid_home 'f13a0978-b1b5-1a4e-8821-39438e24b627'
	fw_setenv uuid_panic 'f20aa902-1c5d-294a-9177-97a513e3cae4'
	fw_setenv uuid_rootfs '012b3303-34ac-284d-99b4-34e03a2335f4'
	fw_setenv uuid_uboot0 'd117f98e-6f2c-d04b-a5b2-331a19f91cb2'
	fw_setenv uuid_uboot1 '8a4bb8b4-e304-ae48-8536-aff5c9c495b1'
	fw_setenv uuid_uboot_env0 '25718777-d0ad-7443-9e60-02cb591c9737'
	fw_setenv uuid_uboot_env1 '08992135-13c6-084b-9322-3391ff571e19'
	fw_setenv uuid_update 'faec2ecf-8544-e241-b19d-757e796da607'
}

obsiot_env(){
	fw_setenv env_version 'a'
	fw_setenv rootfs 'root=/dev/ram'
	fw_setenv stdcmd 'run bootDebian'
	fw_setenv bootcmd 'mw.l 0xff008030 0x0;setexpr.b ret *0xff008018 \\& 0x20;if test.b \${ret} != 0;then run stdcmd;fi;'
	fw_setenv bootDebian 'echo "Target:\${target_name}";run chkmodel;run do_partition; run do_handle_bootargs_mode;'
	fw_setenv mmc-bootargs 'setenv bootargs \${rootfs} rw \${bootargs_console} \${bootargs_debug} systemd.unit=\${bootargs_target}.target hardware_id=\${hardware_id} g_multi.iSerialNumber=\${serial#} \${miscargs} \${noflashcfg} \${obsiot}'
	fw_setenv load_kernel 'fatload mmc 0:7 \${loadaddr} bzImage;fatload mmc 0:7 0x8000000 initrd.gz'
	fw_setenv boot_target_cmd 'run do_flash_os;run do_probe_dfu;run do_compute_target;run chkinit;run mmc-bootargs;mw.l 0xff008020 0x00458000;mw.l 0xff008038 0x00018000;run load_kernel;zboot \${loadaddr} 0x0 0x8000000 0x5000000'
	fw_setenv chkinit 'setexpr.b ret *0xff008005 \\& 0x40;if test.b \${ret} = 0;then setenv noflashcfg noflashcfg=1;else setenv noflashcfg;fi;'
	fw_setenv chkmodel 'setexpr.b dip1 *0xff008009 \\& 0x10;if test.b ${dip1} != 0;then setenv obsiot obsiot=bx1;else setenv obsiot obsiot=ex1;fi;'
}

usage(){
	echo "Update uboot environment for OpenBlocks IoT"
	echo
	echo "usage: $(basename $0) [-d]"
	echo
	echo "	-a	OpenBlocks IoT default environment"
	echo "	-h	This messages"
}

if [ "$1" = "-a" ]; then
	obsiot_env
elif [ "$1" = "--edison-default" ]; then
	dd if=/dev/zero of=/dev/mmcblk0p2 bs=64k count=1
	dd if=/dev/zero of=/dev/mmcblk0p4 bs=64k count=1
	default_env
else
	usage
	exit 1
fi

exit 0
