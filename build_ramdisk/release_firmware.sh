#!/bin/bash
. `dirname $0`/config.sh

if [ "$CROSS" == "true" ]; then
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH CROSS_COMPILE=${CROSS_COMPILE=}"
fi

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}

mount -o loop ${_RAMDISK_IMG} /mnt

rm -rf /mnt/lib/modules/${KERNEL}
rm -rf /mnt/lib/firmware

VERSION=$(cat /mnt/etc/openblocks-release)

(cd ${LINUX_SRC}; make INSTALL_MOD_PATH=/mnt ${KERN_COMPILE_OPTS} modules_install)
cp -f ${LINUX_SRC}/System.map /mnt/boot/
cp -f ${LINUX_SRC}/System.map ${RELEASEDIR}
rm -f /mnt/lib/modules/${KERNEL}/source /mnt/lib/modules/${KERNEL}/build

umount /mnt

gzip -9 < ${LINUX_SRC}/arch/arm/boot/zImage > ${RELEASEDIR}/zImage.gz

lzma -${LZMA_LEVEL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/${RAMDISK_IMG}.lzma

mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}${PATCH_LEVEL}" \
	-A arm -O linux -T multi -C gzip -a 0x8000 -e 0x8000 \
	-d ${RELEASEDIR}/zImage.gz:${RELEASEDIR}/${RAMDISK_IMG}.lzma \
	${RELEASEDIR}/uImage.initrd.${TARGET}

(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb.sh ${VERSION} ${ARCH} ${RELEASEDIR}/uImage.initrd.${TARGET})

