#!/bin/bash
#
# Copyright (c) 2013-2022 Plat'Home CO., LTD.
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
#if [ $TARGET == "obsax3" ]; then
#	cp -a ${PWD}/wifi/wheezy/ath10k $MOUNTDIR/lib/firmware
#fi

#echo
#echo "touch all files"
#echo
#find ${MOUNTDIR} | xargs touch -t `date +"%Y%m%d0000"`

depmod -ae -b ${MOUNTDIR} -F ${MOUNTDIR}/boot/System.map ${KERNEL}

umount ${MOUNTDIR}

if [ ! -d ${RELEASEDIR} ]; then
	mkdir -p ${RELEASEDIR}
fi

cp -f ${LINUX_SRC}/System.map ${RELEASEDIR}

if [ "$TARGET" == "obs600" ]; then
	cp -f ${LINUX_SRC}/vmlinux.bin.gz ${RELEASEDIR}
else
	case $KERNEL in
	3.13|4.*)
		cat ${LINUX_SRC}/arch/${KERN_ARCH}/boot/zImage \
			${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/${DTBFILE} \
			> ${RELEASEDIR}/zImage.dtb
	;;
	*)
		cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/zImage ${RELEASEDIR}
	;;
	esac
fi

${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/${RAMDISK_IMG}.${COMP_EXT}

if [ "$TARGET" == "obs600" ]; then
	mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}" \
		-A ppc -O linux -T multi -C gzip \
		-d ${RELEASEDIR}/vmlinux.bin.gz:${RELEASEDIR}/${RAMDISK_IMG}.${COMP_EXT}:${LINUX_SRC}/arch/${KERN_ARCH}/boot/${TARGET}.dtb \
		${RELEASEDIR}/uImage.initrd.${TARGET}
else
	case $KERNEL in
	3.13|4.*)
		mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}" \
			-A arm -O linux -T multi -C none -a 0x8000 -e 0x8000 \
			-d ${RELEASEDIR}/zImage.dtb:${RELEASEDIR}/${RAMDISK_IMG}.${COMP_EXT} \
			${RELEASEDIR}/uImage.initrd.${TARGET}
	;;
	*)
		mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}" \
			-A arm -O linux -T multi -C none -a 0x8000 -e 0x8000 \
			-d ${RELEASEDIR}/zImage:${RELEASEDIR}/${RAMDISK_IMG}.${COMP_EXT} \
			${RELEASEDIR}/uImage.initrd.${TARGET}
	;;
	esac
fi
(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/uImage.initrd.${TARGET})

if [ "${TARGET}" != "obs600" ]; then
	ARCHIVE=kernel+modules-${KERNEL}-${PATCHLEVEL}
	TMP=${RELEASEDIR}/${ARCHIVE}
	rm -fr ${TMP}
	mkdir ${TMP}
	cp -f ${LINUX_SRC}/System.map ${TMP}/System.map
	cp -f ${LINUX_SRC}/System.map ${TMP}/System.map.$(echo ${KERNEL}|tr -d .)
	case $KERNEL in
	3.13|4.*)
		cp -f ${RELEASEDIR}/zImage.dtb ${TMP}
		;;
	*)
		cp -f ${RELEASEDIR}/zImage ${TMP}
		;;
	esac
	mount -o loop,ro ${_RAMDISK_IMG} ${MOUNTDIR}
	if [ -d ${MOUNTDIR}/lib/firmware ]; then
		FIRMWARE="lib/firmware"
	fi
	if [ -d ${MOUNTDIR}/lib/modules ]; then
		MODULES="lib/modules"
	fi
	(cd ${MOUNTDIR}; tar cpf - $FIRMWARE $MODULES) | (cd ${TMP}; tar xpf -)
	umount ${MOUNTDIR}
	find ${TMP}/lib/modules -name "*.ko" | xargs gzip -9f
	(cd ${RELEASEDIR}; tar cpf ${RELEASEDIR}/${ARCHIVE}.tar ${ARCHIVE})
	xz -9f ${RELEASEDIR}/${ARCHIVE}.tar
	rm -fr ${TMP}
fi

(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
