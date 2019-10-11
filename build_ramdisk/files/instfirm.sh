#!/bin/bash
#
# instfirm
#
### BEGIN INIT INFO
# Provides:          instfirm
# Required-Start:
# Required-Stop:
# X-Start-Before:    easyblockssync easyblocks-standalone
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: install firmware
# Description:       install firmware for OBSVX1
### END INIT INFO
NAME=instfirm
PATH=/sbin:/bin:/usr/sbin:/usr/bin

#includes lsb functions 
. /lib/lsb/init-functions

EBDIR=/etc/easyblocks
[ -f ${EBDIR}/lib/common ] && . ${EBDIR}/lib/common

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

set -e

#debug=echo

MODESTR="instfirm=debian"

case "$1" in
start)
	grep -qv $MODESTR /proc/cmdline && exit

	BOOT=`findfs LABEL=${FIRM_DIR} 2> /dev/null`
	if [ -z "$BOOT" ]; then
		echo "Boot partition is not found."
		exit 1
	fi

	# format partition
	if [ -e ${BOOT/%?/}2 ]; then
		wipefs -a ${BOOT/%?/}2
		mkfs.ext4 -U e8c3e922-b1f5-43a2-a026-6a14f01197f6 ${BOOT/%?/}2
	fi

	mount $BOOT /media || exit 1
	cp /media/SFR/${MODEL}-bzImage /media/bzImage
	if [ "$MODEL" == "obsvx1 ]; then
		cp /media/SFR/obsvx1-initrd.gz /media/initrd.gz
		e2label ${BOOT/%?/}2 DEBIAN
	fi
	cp /media/SFR/openblocks-release /media/openblocks-release
	cp /media/EFI/boot/bootx64.conf-obsiot /media/EFI/boot/bootx64.conf
	if [ -f /media/SFR/${MODEL}-rootfs.tgz ]; then
		mount ${BOOT/%?/}2 /mnt || exit 1
		tar xfzp /media/SFR/${MODEL}-rootfs.tgz -C /mnt
		depmod -ae -b /mnt -F /mnt/boot/System.map `cat /mnt/etc/openblocks-release | cut -d - -f1`
		sync
		umount /mnt
	fi
	if [ -f /media/SFR/${MODEL}_userland.tgz ]; then
		mount ${BOOT/%?/}2 /mnt || exit 1
		tar xfzp /media/SFR/${MODEL}_userland.tgz -C /mnt
		sync
		umount /mnt
	fi
	rm -f /media/etc.tgz /media/userland.tgz
	sync
	umount /media
	reboot
	;;
stop)
	;;
*)
	echo "Usage: /etc/init.d/$NAME {start}" >&2
	exit 1
	;;
esac

exit 0
