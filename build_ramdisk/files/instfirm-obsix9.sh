#!/bin/bash
#
# Copyright (c) 2013-2018 Plat'Home CO., LTD.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY PLAT'HOME CO., LTD. AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL PLAT'HOME CO., LTD. AND CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

set -e

#debug=echo

MODESTR="recoverymodel"
grep -qv $MODESTR /proc/cmdline && exit

#
# replace MODEL name
#
ary=(`cat /proc/cmdline`)
for i in `seq 1 ${#ary[@]}`
do
	case ${ary[$i]} in
	${MODESTR}*)
		IFS='='
		set -- ${ary[$i]}
		MODEL=$2
		;;
	esac
done

[ "$MODEL" != "obsix9" && "$MODEL" != "obsix9r" ] && exit 1

BOOT=`findfs LABEL=${FIRM_DIR} 2> /dev/null`
if [ -z "$BOOT" ]; then
	echo "Boot partition is not found."
	exit 1
fi

# format rootfs partition
wipefs -a ${BOOT}
mkfs.ext4 ${BOOT}

mount /dev/mmcblk0p1 /media || exit 1
# copy kernel image
cp /media/SFR/openblocks-release /media/openblocks-release
# copy ramdisk image
if [ "$MODEL" == "obsix9r" ]; then
	cp /media/SFR/obsix9r-bzImage /media/bzImage
	cp /media/SFR/obsix9r-initrd.gz /media/initrd.gz
	e2label /dev/mmcblk0p2 DEBIAN
	cp /media/EFI/boot/bootx64.conf-obsix9r /media/EFI/boot/bootx64.conf
else
	cp /media/SFR/obsix9-bzImage /media/bzImage
	cp /media/EFI/boot/bootx64.conf-obsix9 /media/EFI/boot/bootx64.conf
	mount ${BOOT} /mnt || exit 1
	tar xfzp /media/SFR/obsix9-rootfs.tgz -C /mnt
	depmod -ae -b /mnt -F /mnt/boot/System.map `cat /mnt/etc/openblocks-release | cut -d - -f1`
	if [ -f /media/SFR/obsix9_userland.tgz ]; then
		tar xfzp /media/SFR/${MODEL}_userland.tgz -C /mnt
	fi
	sync
	umount /mnt
fi

rm -f /media/etc.tgz /media/userland.tgz
sync
umount /media
reboot

exit 0
