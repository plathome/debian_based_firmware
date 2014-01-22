#!/bin/bash
#
# Copyright (c) 2013, 2014 Plat'Home CO., LTD.
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

. `dirname $0`/config.sh

if [ "$CROSS" == "true" ]; then
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH CROSS_COMPILE=${CROSS_COMPILE=}"
	if [ "${KERNEL}" == "2.6.31" ] ||
	   [ "${TARGET}" == "obs600" -a "${DIST}" == "wheezy" ]
	then
		KERN_COMPILE_OPTS+=" NOGZIP=1"
	fi
fi

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}

mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}

rm -rf ${MOUNTDIR}/lib/modules/${KERNEL}
rm -rf ${MOUNTDIR}/lib/firmware

VERSION=$(cat ${MOUNTDIR}/etc/openblocks-release)

(cd ${LINUX_SRC}; make INSTALL_MOD_PATH=${MOUNTDIR} ${KERN_COMPILE_OPTS} modules_install)
cp -f ${LINUX_SRC}/System.map ${MOUNTDIR}/boot/
rm -f ${MOUNTDIR}/lib/modules/${KERNEL}/source ${MOUNTDIR}/lib/modules/${KERNEL}/build
if [ "${KERNEL}" == "2.6.31" -o "${TARGET}" == "obs600" ]; then
	rm -f ${MOUNTDIR}/lib/modules/${KERNEL}/modules.builtin.bin
fi

umount ${MOUNTDIR}

if [ ! -d ${RELEASEDIR} ]; then
	mkdir -p ${RELEASEDIR}
fi

cp -f ${LINUX_SRC}/System.map ${RELEASEDIR}

if [ "$TARGET" == "obs600" ]; then
	cp -f ${LINUX_SRC}/vmlinux.bin.gz ${RELEASEDIR}
elif [ "$TARGET" == "obsa7" ]; then
	cat ${LINUX_SRC}/arch/${KERN_ARCH}/boot/zImage ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/${DTBFILE} > ${RELEASEDIR}/zImage.dtb
else
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/zImage ${RELEASEDIR}
fi

${COMPRESS} -${LZMA_LEVEL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/${RAMDISK_IMG}.${COMPRESS_EXT}

if [ "$TARGET" == "obs600" ]; then
mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}${PATCH_LEVEL}" \
	-A ppc -O linux -T multi -C gzip \
	-d ${RELEASEDIR}/vmlinux.bin.gz:${RELEASEDIR}/${RAMDISK_IMG}.${COMPRESS_EXT}:${LINUX_SRC}/arch/${KERN_ARCH}/boot/${TARGET}.dtb \
	${RELEASEDIR}/uImage.initrd.${TARGET}
(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb.sh ${VERSION} ${ARCH} ${RELEASEDIR}/uImage.initrd.${TARGET})
elif [ "$TARGET" == "obsa7" ]; then
mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}${PATCH_LEVEL}" \
	-A arm -O linux -T multi -C none -a 0x8000 -e 0x8000 \
	-d ${RELEASEDIR}/zImage.dtb:${RELEASEDIR}/${RAMDISK_IMG}.${COMPRESS_EXT} \
	${RELEASEDIR}/uImage.initrd.${TARGET}
(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb.sh ${VERSION} ${ARCH} ${RELEASEDIR}/uImage.initrd.${TARGET})
else
mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}${PATCH_LEVEL}" \
	-A arm -O linux -T multi -C none -a 0x8000 -e 0x8000 \
	-d ${RELEASEDIR}/zImage:${RELEASEDIR}/${RAMDISK_IMG}.${COMPRESS_EXT} \
	${RELEASEDIR}/uImage.initrd.${TARGET}

(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb.sh ${VERSION} ${ARCH} ${RELEASEDIR}/uImage.initrd.${TARGET})
fi

(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
