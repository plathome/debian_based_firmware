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

usage()
{
	echo -e "$0 src dst [model]"
	echo -e "src : Partition of source storage (exp. USB memory is /dev/sda)"
	echo -e "dst : Distnation storage (exp. eMMC is= /dev/mmcblk0)"
	echo -e "[model] : obsvx1 or obsvx2 (default is obsvx1)"
	return 0
}

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

src=$1
dist=$2
target=$3

case $MODEL in
obsvx*)
	[ -z $target ] && target="obsvx1"
	;;
obsix*)
	[ -z $target ] && target="obsix9"
	;;
*)
	echo -e "$MODEL is not supported"
	echo
	usage
	exit 1
	;;
esac

if [ ! -e ${dist} ]; then
	usage
	exit 1
fi
if [ ! -e ${src}1 ]; then
	usage
	exit 1
fi

# remove partitions
for num in 1 2 3; do
	if [ -e ${dist}p$num ]; then
		parted ${dist} -s rm $num
	fi
done

# make partition
parted ${dist} -s mklabel gpt
parted ${dist} -s mkpart boot fat16 1M 1537M
parted ${dist} -s mkpart primary ext4 1537M 100%
sleep 1

# remove partitions info
wipefs -a ${dist}p1
wipefs -a ${dist}p2

# format partition
mkfs.vfat -n BOOT ${dist}p1
#mkfs.ext4 -L DEBIAN -U e8c3e922-b1f5-43a2-a026-6a14f01197f6 ${dist}p2
mkfs.ext4 -U e8c3e922-b1f5-43a2-a026-6a14f01197f6 ${dist}p2

# copy partition
mount ${src}1 /media || exit 1

# boot partition
mount ${dist}p1 /mnt || exit 1
( cd /media; tar cfpm - . | tar xfpm - -C /mnt )
cp /mnt/EFI/boot/bootx64.conf-obsiot /mnt/EFI/boot/bootx64.conf
cp /mnt/SFR/${target}-bzImage /mnt/bzImage
if [ "$target" == "obsvx1" -o "$target" == "obsix9" ]; then
	if [ -f /mnt/SFR/${target}-initrd.gz ]; then
		cp /mnt/SFR/${target}-initrd.gz /mnt/initrd.gz
	else
		if [ -f /mnt/SFR/${target}r-initrd.gz ]; then
			cp /mnt/SFR/${target}r-initrd.gz /mnt/initrd.gz
		else
			echo
			echo "/mnt/SFR/${target}-initrd.gz is no found."
			echo
		fi
	fi
fi
sync
umount /mnt

# restore RootFS and WebUI
mount ${dist}p2 /mnt || exit 1
if [ -f /media/SFR/${target}_userland.tgz ]; then
	tar xfzp /media/SFR/${target}_userland.tgz -C /mnt
elif [ -f /media/SFR/${target}-rootfs.tgz ]; then
	tar xfzp /media/SFR/${target}-rootfs.tgz -C /mnt
fi
sync
umount /mnt

umount /media

echo "done."
exit 0
