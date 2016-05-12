#!/bin/bash
#
# Copyright (c) 2013-2016 Plat'Home CO., LTD.
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
else
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH"
fi

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}

mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}

rm -rf ${MOUNTDIR}/lib/modules/${KERNEL}
rm -rf ${MOUNTDIR}/lib/firmware

VERSION=${KERNEL}-${PATCHLEVEL}

(cd ${LINUX_SRC}; make INSTALL_MOD_PATH=${MOUNTDIR} ${KERN_COMPILE_OPTS} modules_install)
cp -f ${LINUX_SRC}/System.map ${MOUNTDIR}/boot/
if [ "$TARGET" == "obsbx1" ]; then
	echo "8812AU"
	if [ ${KERNEL} == "3.10.17" ]; then
		LOCAL_VER="-poky-edison"
	else
		LOCAL_VER=""
	fi
	if [ -d ${FILESDIR}/rtl8812AU_8821AU_linux-master ]; then
		(cd ${FILESDIR}/rtl8812AU_8821AU_linux-master;	\
			CFLAGS="-m32" LDFLAGS="-m32" CC=gcc KERNELPATH=${LINUX_SRC} make; \
			mkdir -p ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless;	\
			INSTALLPATH=${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless make install)
	fi

	mkdir -p ${MOUNTDIR}/factory
	rm -f ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/source ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/build
else
	rm -f ${MOUNTDIR}/lib/modules/${KERNEL}/source ${MOUNTDIR}/lib/modules/${KERNEL}/build
fi

if [ "$TARGET" == "obsbx1" ]; then
	depmod -ae -b ${MOUNTDIR} -F ${MOUNTDIR}/boot/System.map ${KERNEL}${LOCAL_VER}
else
	depmod -ae -b ${MOUNTDIR} -F ${MOUNTDIR}/boot/System.map ${KERNEL}
fi

umount ${MOUNTDIR}

if [ ! -d ${RELEASEDIR} ]; then
	mkdir -p ${RELEASEDIR}
fi

cp -f ${LINUX_SRC}/System.map ${RELEASEDIR}

case $TARGET in
bpv4*|bpv8)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}
	${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/ramdisk-bpv.img.${COMPEXT}
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}/bzImage.recovery
	cp -f ${RELEASEDIR}/ramdisk-bpv.img.${COMPEXT} ${RELEASEDIR}/ramdisk-bpv.img.recovery.${COMPEXT}
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/ramdisk-bpv.img.${COMPEXT} ${FILESDIR}/grub/grub.cfg ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} dummy)
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
;;
obsbx1)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}
	case $DIST in
	wheezy)
		${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/${RAMDISK_IMG}.${COMPEXT}
		(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
		(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/${RAMDISK_IMG}.${COMPEXT} ${FILESDIR}/grub/grub.cfg ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} ${FILESDIR})
	;;
	jessie)
		${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMPEXT}
		(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
		(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/initrd.${COMPEXT} ${FILESDIR}/grub/grub.cfg ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} ${FILESDIR})
	;;
	esac
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
;;
esac
