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

MODESTR="instfirm=debian"
RECOVERY="recoverymodel"

grep -qv $MODESTR /proc/cmdline && exit

# replace MODEL name
ary=(`cat /proc/cmdline`)
for i in `seq 1 ${#ary[@]}`
do
	case ${ary[$i]} in
	${RECOVERY}*)
		IFS='='
		set -- ${ary[$i]}
		MODEL=$2
		;;
	esac
done

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
if [ "$MODEL" == "obsvx1" ]; then
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

exit 0
