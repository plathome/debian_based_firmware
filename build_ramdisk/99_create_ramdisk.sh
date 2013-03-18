#!/bin/bash

. `dirname $0`/config.sh

size=${RAMDISK_SIZE:=160}

cd ${WRKDIR}

umount /mnt

rm -f ${RAMDISK_IMG}

dd if=/dev/zero of=${RAMDISK_IMG} count=0 seek=$(($size * 1024 * 1024 / 512))

echo y | mke2fs ${RAMDISK_IMG}

mkdir -p /mnt
mount -o loop ${RAMDISK_IMG} /mnt

(cd ${SQUEEZEDIR};tar --numeric-owner -cpf - . | tar -xvf - -C /mnt)

umount /mnt

tune2fs -c 0 -i 0 ${RAMDISK_IMG}

#(dd if=${RAMDISK_IMG} | gzip -9 > ${RAMDISK_IMG}.gz)
