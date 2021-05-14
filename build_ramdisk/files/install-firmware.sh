#!/bin/bash
#
# Copyright (c) 2013-2021 Plat'Home CO., LTD.
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

usage()
{
	echo -e "$0 src model"
	echo -e "src : Partition of source storage (exp. USB memory is /dev/sda1)"
	echo -e "model : obsvx1, obsvx2, obsix9 or obsix9r"
	return 0
}

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

src=$1
target=$2
if [ ! -e ${src} ]; then
	usage
	exit 1
fi

case ${target} in
obsvx1|obsvx2|obsix9|obsix9r)
	;;
*)
	usage
	exit 1
	;;
esac

# remove partitions
for num in 1 2 3; do
	if [ -e /dev/mmcblk0p$num ]; then
		parted /dev/mmcblk0 -s rm $num
	fi
done

# make partition
parted /dev/mmcblk0 -s mklabel gpt
parted /dev/mmcblk0 -s mkpart boot fat16 1M 1537M
parted /dev/mmcblk0 -s mkpart primary ext4 1537M 100%
sleep 1

# remove partitions info
wipefs -a /dev/mmcblk0p1
wipefs -a /dev/mmcblk0p2

# format partition
mkfs.vfat -n BOOT /dev/mmcblk0p1
mkfs.ext4 -U e8c3e922-b1f5-43a2-a026-6a14f01197f6 /dev/mmcblk0p2

# copy partition
mount ${src} /media || exit 1

# boot partition
mount /dev/mmcblk0p1 /mnt || exit 1
( cd /media; tar cfpm - . | tar xfpm - -C /mnt )
cp /mnt/EFI/boot/bootx64.conf-obsiot /mnt/EFI/boot/bootx64.conf
cp /mnt/SFR/${target}-bzImage /mnt/bzImage
if [ "${target}" == "obsvx1" -o "${target}" == "obsix9r" ]; then
	if [ -f /mnt/SFR/${target}-initrd.gz ]; then
		cp /mnt/SFR/${target}-initrd.gz /mnt/initrd.gz
	else
		echo
		echo "initrd.gz is not found."
		exit 1
	fi
fi
sync
umount /mnt

# restore RootFS and WebUI
if [ "${target}" == "obsvx2" -o "${target}" == "obsix9" ]; then
	mount /dev/mmcblk0p2 /mnt || exit 1
	if [ -f /media/SFR/${target}_userland.tgz ]; then
		tar xfzp /media/SFR/${target}_userland.tgz -C /mnt
	elif [ -f /media/SFR/${target}-rootfs.tgz ]; then
		tar xfzp /media/SFR/${target}-rootfs.tgz -C /mnt
	else
		echo
		echo "userland or rootfs file is not found."
		exit 1
	fi
fi
sync
umount /mnt
umount /media

echo "done."
exit 0
