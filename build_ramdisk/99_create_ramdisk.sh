#!/bin/bash

. `dirname $0`/config.sh

size=${RAMDISK_SIZE:=160}

cd ${WRKDIR}

umount ${MOUNTDIR}

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}
rm -f ${_RAMDISK_IMG}

dd if=/dev/zero of=${_RAMDISK_IMG} count=0 seek=$(($size * 1024 * 1024 / 512))

echo y | mke2fs ${_RAMDISK_IMG}

mkdir -p ${MOUNTDIR}
mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}

(cd ${DISTDIR};tar --numeric-owner --exclude=${QEMU_BIN} -cpf - . | tar -xvf - -C ${MOUNTDIR})

umount ${MOUNTDIR}

tune2fs -c 0 -i 0 ${_RAMDISK_IMG}

