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

. `dirname $0`/config.sh

if [ "$CROSS" == "true" ]; then
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH CROSS_COMPILE=${CROSS_COMPILE=}"
else
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH"
fi
case $TARGET in
obsgem1) KERN_COMPILE_OPTS=" $KERN_COMPILE_OPTS INSTALL_MOD_STRIP=1" ;;
esac

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}

mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}

rm -rf ${MOUNTDIR}/lib/modules/${KERNEL}
rm -rf ${MOUNTDIR}/lib/firmware

VERSION=${KERNEL}-${PATCHLEVEL}

(cd ${LINUX_SRC}; make INSTALL_MOD_PATH=${MOUNTDIR} ${KERN_COMPILE_OPTS} modules_install)
cp -f ${LINUX_SRC}/System.map ${MOUNTDIR}/boot/

case "$TARGET" in
obsbx*)
	case ${KERNEL} in
	3.10.*)
		echo "8812AU"
		LOCAL_VER="-poky-edison"
		if [ -d ${FILESDIR}/rtl8812AU_8821AU_linux-master ]; then
			(cd ${FILESDIR}/rtl8812AU_8821AU_linux-master;	\
				CFLAGS="-m32" LDFLAGS="-m32" CC=gcc KERNELPATH=${LINUX_SRC} make; \
				mkdir -p ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless;	\
				INSTALLPATH=${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless make install)
		fi
		;;
	4.4.*)
		LOCAL_VER="-brillo-edison"
#		if [ -d ${FILESDIR}/rtl8812AU_8821AU_linux-master ]; then
#			(cd ${FILESDIR}/rtl8812AU_8821AU_linux-master;	\
#				CFLAGS="-m32" LDFLAGS="-m32" CC=gcc KERNELPATH=${LINUX_SRC} make; \
#				mkdir -p ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless;	\
#				INSTALLPATH=${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless make install)
#		fi
		;;
	*)
		LOCAL_VER=""
		;;
	esac
	mkdir -p ${MOUNTDIR}/factory
	;;
obsvx*)
	case ${KERNEL} in
#	4.19.*)
#		LOCAL_VER="-cip1-rt1"
#		;;
	*) ;;
	esac
	if [ -d ${FILESDIR}/rtl8821ae ]; then
		if [ "$KERNEL" != "4.19.37" ]; then
		echo "8821AE"
		(cd ${FILESDIR}/rtl8821ae; \
			mkdir -p ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless/realtek;	\
			make all install KSRC=${LINUX_SRC} KVER=${KERNEL} MODDESTDIR=${MOUNTDIR}/lib/modules/${KERNEL}/kernel/drivers/net/wireless/realtek MOUNTDIR=${MOUNTDIR} ${COMPRESS_XZ} USER_EXTRA_CFLAGS="-Wno-error=date-time -fno-pic -Wno-pointer-sign")
		fi
	fi
	;;
obsgem1)
	LOCAL_VER="-qcomlt-arm64"
	;;
*)
	;;
esac
rm -f ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/source ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/build

case $TARGET in
obsgem1)
	if [ -d ${FILESDIR}/firmware-${TARGET}-${KERNEL} ]; then
		mkdir -p ${MOUNTDIR}/lib/firmware
		cp -a ${FILESDIR}/firmware-${TARGET}-${KERNEL}/* ${MOUNTDIR}/lib/firmware
	fi
	;;
*)
	if [ -d ${FILESDIR}/firmware-${TARGET} ]; then
		mkdir -p ${MOUNTDIR}/lib/firmware
		cp -a ${FILESDIR}/firmware-${TARGET}/* ${MOUNTDIR}/lib/firmware
	fi
	;;
esac

	depmod -ae -b ${MOUNTDIR} -F ${MOUNTDIR}/boot/System.map ${KERNEL}${LOCAL_VER}

if [ ! -d ${RELEASEDIR} ]; then
	mkdir -p ${RELEASEDIR}
fi

if [ "$TARGET" == "obsvx2" -o "$TARGET" == "obsgem1" ]; then
	# kernel modules and firmware
	(cd ${MOUNTDIR}/lib; tar cfzp ${RELEASEDIR}/modules.tgz firmware modules)
elif [ "$TARGET" == "obsbx1s" ]; then
	# kernel modules and firmware
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/modules.tgz etc/firmware lib/modules)
fi

umount ${MOUNTDIR}

cp -f ${LINUX_SRC}/System.map ${RELEASEDIR}

case $TARGET in
obsvx1)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}
	${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMPEXT}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/initrd.${COMPEXT} dummy ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} dummy)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsvx2)
	# obs tools
	USRSBIN=${DISTDIR}/usr/sbin
	OBSTOOLS="${USRSBIN}/wd-keepalive ${USRSBIN}/obs-util ${USRSBIN}/kosanu ${USRSBIN}/runled ${USRSBIN}/pshd ${USRSBIN}/atcmd ${USRSBIN}/obs-hwclock ${USRSBIN}/hub-ctrl ${USRSBIN}/wav-play ${USRSBIN}/obsvx1-modem ${USRSBIN}/obsvx1-gpio ${USRSBIN}/obsiot-power"
	ETCINITD=${DISTDIR}/etc/init.d
	OBSSCRIPTS="${ETCINITD}/obsiot-power"
	ETCUDEVRULES=${DISTDIR}/etc/udev/rules.d
	OBSUDEVRULES="${ETCUDEVRULES}/50-obsvx1-symlink-ttyMODEM.rules"
	WORK=/tmp/_tmpfs.$$
	mkdir -p ${WORK}/usr/sbin
	mkdir -p ${WORK}/etc/init.d
	mkdir -p ${WORK}/etc/udev/rules.d
	cp -f ${OBSTOOLS} ${WORK}/usr/sbin
	cp -f ${OBSSCRIPTS} ${WORK}/etc/init.d
	cp -f ${OBSUDEVRULES} ${WORK}/etc/udev/rules.d
	(cd ${WORK}; tar cfzp ${RELEASEDIR}/obstools.tgz .)
	rm -rf ${WORK}

	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/obstools.tgz ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/System.map)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsgem1)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}
	${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMP_EXT}
#	mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}" \
#		-A arm64 -O linux -T kernel -C none -a 0x8008000 -e 0x8008000 \
#		-d ${RELEASEDIR}/Image.gz \
#		${RELEASEDIR}/uImage.${TARGET}
#	mkimage -n "$(echo ${TARGET}|tr [a-z] [A-Z]) ${VERSION}" \
#		-A arm64 -T ramdisk -C gzip \
#		-d ${RELEASEDIR}/initrd.${COMP_EXT} \
#		${RELEASEDIR}/uInitrd.${TARGET}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/${MAKE_IMAGE} ${RELEASEDIR}/initrd.${COMP_EXT} dummy ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} dummy)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})

	# obs tools
	USRSBIN=${DISTDIR}/usr/sbin
	OBSTOOLS="${USRSBIN}/wd-keepalive ${USRSBIN}/obs-util ${USRSBIN}/kosanu ${USRSBIN}/runled ${USRSBIN}/pshd ${USRSBIN}/atcmd ${USRSBIN}/obs-hwclock ${USRSBIN}/wav-play ${USRSBIN}/obsiot-power"
	ETCINITD=${DISTDIR}/etc/init.d
	OBSSCRIPTS="${ETCINITD}/obsiot-power"
	WORK=/tmp/_tmpfs.$$
	mkdir -p ${WORK}/usr/sbin
	mkdir -p ${WORK}/etc/init.d
	cp -f ${OBSTOOLS} ${WORK}/usr/sbin
	cp -f ${OBSSCRIPTS} ${WORK}/etc/init.d
	(cd ${WORK}; tar cfzp ${RELEASEDIR}/obstools.tgz .)
	rm -rf ${WORK}

	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/${MAKE_IMAGE} ${RELEASEDIR}/obstools.tgz ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/System.map)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
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
	jessie|stretch|buster)
		${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMPEXT}
		(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
		(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/initrd.${COMPEXT} ${FILESDIR}/grub/grub.cfg ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} ${FILESDIR})
	;;
	esac
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
;;
obsbx1s)
	# obs tools
	USRSBIN=${DISTDIR}/usr/sbin
	OBSTOOLS="${USRSBIN}/wd-keepalive ${USRSBIN}/obs-util ${USRSBIN}/kosanu ${USRSBIN}/runled ${USRSBIN}/pshd ${USRSBIN}/atcmd ${USRSBIN}/obs-hwclock ${USRSBIN}/hub-ctrl ${USRSBIN}/wav-play ${USRSBIN}/obsiot-power"
	ETCINITD=${DISTDIR}/etc/init.d
	OBSSCRIPTS="${ETCINITD}/obsiot-power ${ETCINITD}/nitz"
	ETCUDEVRULES=${DISTDIR}/etc/udev/rules.d
	OBSUDEVRULES="${ETCUDEVRULES}/50-obsbx1-symlink-ttyMODEM.rules ${ETCUDEVRULES}/40-rename-ttyrs485.rules "
	WORK=/tmp/_tmpfs.$$
	mkdir -p ${WORK}/usr/sbin
	mkdir -p ${WORK}/etc/init.d
	mkdir -p ${WORK}/etc/udev/rules.d
	cp -f ${OBSTOOLS} ${WORK}/usr/sbin
	cp -f ${OBSSCRIPTS} ${WORK}/etc/init.d
	cp -f ${OBSUDEVRULES} ${WORK}/etc/udev/rules.d
	(cd ${WORK}; tar cfzp ${RELEASEDIR}/obstools.tgz .)
	rm -rf ${WORK}

	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	# uboot env update script
	cp -f ${FILESDIR}/update_ubootenv-${DIST}.sh ${RELEASEDIR}/update_ubootenv.sh

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/obstools.tgz ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/System.map)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
esac
