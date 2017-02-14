#!/bin/bash
#
# Copyright (c) 2013-2017 Plat'Home CO., LTD.
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
	echo -e "$0 src dst"
	echo -e "src : Partition of source storage (exp. USB memory is /dev/sda)"
	echo -e "dst : Distnation storage (exp. eMMC is= /dev/mmcblk0)"
	return 0
}

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

src=$1
dist=$2

if [ ${MODEL} != "obsvx1" ]; then
	usage
	exit 1
fi
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
parted ${dist} -s mkpart primary ext4 1537M 7291M
parted ${dist} -s mkpart swap ext4 7291M 7803M
sleep 1

# remove partitions info
wipefs -a ${dist}p1
wipefs -a ${dist}p2
wipefs -a ${dist}p3

# format partition
mkfs.vfat -n BOOT ${dist}p1
mkfs.ext4 -L DEBIAN -U e8c3e922-b1f5-43a2-a026-6a14f01197f6 ${dist}p2
#mkfs.ext4 ${dist}p3	# swap partition is unused, format ext4

# copy partition
mount ${src}1 /media || exit 1

## boot partition
mount ${dist}p1 /mnt || exit 1
( cd /media; tar cfpm - . | tar xfpm - -C /mnt )
cp /mnt/EFI/boot/bootx64.conf-obsiot /mnt/EFI/boot/bootx64.conf
sync
umount /mnt

# restore WebUI
if [ -f /media/SFR/${MODEL}_userland.tgz ]; then
	mount ${dist}p2 /mnt || exit 1
	tar xfzp /media/SFR/${MODEL}_userland.tgz -C /mnt
	sync
	umount /mnt
fi

umount /media

echo "done."
exit 0
