#!/bin/bash
#
# Copyright (c) 2013-2023 Plat'Home CO., LTD.
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

get_kern_ver(){
	echo $(echo "$KERNEL" | (IFS=. read -r major minor micro; printf "%2d%02d%03d" ${major:-0} ${minor:-0} ${micro:-0}))
}

if [ "$CROSS" == "true" ]; then
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH CROSS_COMPILE=${CROSS_COMPILE=}"
else
	KERN_COMPILE_OPTS="ARCH=$KERN_ARCH"
fi
case $TARGET in
obsa16*) KERN_COMPILE_OPTS=" $KERN_COMPILE_OPTS INSTALL_MOD_STRIP=1" ;;
obsfx0*|obsfx1*) KERN_COMPILE_OPTS=" $KERN_COMPILE_OPTS INSTALL_MOD_STRIP=1 KLIB_BUILD=${LINUX_SRC} KLIB=${MOUNTDIR} " ;;
obstb3n*) KERN_COMPILE_OPTS=" $KERN_COMPILE_OPTS INSTALL_MOD_STRIP=1" ;;
esac

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}

mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}

rm -rf ${MOUNTDIR}/lib/modules/${KERNEL}
case $TARGET in
obshx1*|obshx2*)
	;;
*)
	rm -rf ${MOUNTDIR}/lib/firmware
	;;
esac

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
	4.16.*)
		LOCAL_VER="-edison-standard"
		;;
	*)
		LOCAL_VER=""
		;;
	esac
	mkdir -p ${MOUNTDIR}/factory
	;;
obsvx*)
	if [ -d ${FILESDIR}/rtl8821ae ]; then
		if [ `get_kern_ver` -lt 419085 ]; then
		echo "8821AE"
		(cd ${FILESDIR}/rtl8821ae; \
			mkdir -p ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/kernel/drivers/net/wireless/realtek;	\
			make all install KSRC=${LINUX_SRC} KVER=${KERNEL} MODDESTDIR=${MOUNTDIR}/lib/modules/${KERNEL}/kernel/drivers/net/wireless/realtek MOUNTDIR=${MOUNTDIR} ${COMPRESS_XZ} USER_EXTRA_CFLAGS="-Wno-error=date-time -fno-pic -Wno-pointer-sign")
		fi
	fi
	;;
obsfx0*|obsfx1*)
	if [ x"${WIFI_DRIVER}" != x"" ] ; then
		DRIVER_SRC="`dirname ${LINUX_SRC}`/${WIFI_DRIVER}"
		DRIVER_DEFCONFIG=${WIFI_DEFCONFIG}

		if [ ! -d ${DRIVER_SRC} ]; then
			echo "${DRIVER_SRC} is not found."
			exit 1
		fi
		if [ ! -f ${LINUX_SRC}/vmlinux ]; then
			echo "Linux kernel build is not complete."
			exit 1
		fi
		if [ -f "${DRIVER_SRC}/.debian_version" ]; then
			BUILD_DEBIAN_RELEASE=`cat ${DRIVER_SRC}/.debian_version | sed -e 's/\..*$$//'`
			[ "$DEBIAN_RELEASE" != "$BUILD_DEBIAN_RELEASE" ] && (cd ${DRIVER_SRC}; make ${KERN_COMPILE_OPTS} mrproper)
		fi
		cp /etc/debian_version ${DRIVER_SRC}/.debian_version
		(cd ${DRIVER_SRC}; [ ! -f .config ] && make defconfig-${WIFI_DEFCONFIG} ${KERN_COMPILE_OPTS})
		(cd ${DRIVER_SRC}; make ${KERN_COMPILE_OPTS})
		(cd ${DRIVER_SRC}; make modules_install ${KERN_COMPILE_OPTS} INSTALL_MOD_PATH=${MOUNTDIR})
	fi
	;;
*)
	;;
esac
rm -f ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/source ${MOUNTDIR}/lib/modules/${KERNEL}${LOCAL_VER}/build

if [ -d ${FILESDIR}/firmware-${TARGET} ]; then
	mkdir -p ${MOUNTDIR}/lib/firmware
	cp -a ${FILESDIR}/firmware-${TARGET}/* ${MOUNTDIR}/lib/firmware
fi

if [ -d ${FILESDIR}/firmware-${TARGET}-${DIST} ]; then
	mkdir -p ${MOUNTDIR}/lib/firmware
	cp -a ${FILESDIR}/firmware-${TARGET}-${DIST}/* ${MOUNTDIR}/lib/firmware
fi

	depmod -ae -b ${MOUNTDIR} -F ${MOUNTDIR}/boot/System.map ${KERNEL}${LOCAL_VER}

if [ ! -d ${RELEASEDIR} ]; then
	mkdir -p ${RELEASEDIR}
fi

	# kernel modules and firmware
case $TARGET in
obsvx2)
	(cd ${MOUNTDIR}/lib; tar cfzp ${RELEASEDIR}/modules.tgz firmware modules)
	;;
obsbx1s)
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/modules.tgz etc/firmware lib/modules)
	;;
obsix9|obsa16|obsfx0|obsfx1|obstb3n|obshx1|obshx2)
	(cd ${MOUNTDIR}/lib; tar cfzp ${RELEASEDIR}/modules.tgz modules)
	;;
esac

umount ${MOUNTDIR}

cp -f ${LINUX_SRC}/System.map ${RELEASEDIR}

case $TARGET in
obsvx1)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}
	${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMPEXT}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/initrd.${COMPEXT} dummy ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} dummy)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsvx2)
	case $DIST in
	stretch)
		# obs tools
		USRSBIN=${DISTDIR}/usr/sbin
		OBSTOOLS="${USRSBIN}/wd-keepalive ${USRSBIN}/obs-util ${USRSBIN}/kosanu ${USRSBIN}/runled ${USRSBIN}/pshd ${USRSBIN}/atcmd ${USRSBIN}/obs-hwclock ${USRSBIN}/wav-play ${USRSBIN}/obsiot-power ${USRSBIN}/obsiot-modem.sh ${USRSBIN}/obsvx1-modem ${USRSBIN}/obsvx1-gpio"
		ETCINITD=${DISTDIR}/etc/init.d
		OBSSCRIPTS="${ETCINITD}/obsiot-power ${ETCINITD}/nitz ${ETCINITD}/disable-modem"
		ETCUDEVRULES=${DISTDIR}/etc/udev/rules.d
		OBSUDEVRULES="${ETCUDEVRULES}/50-obsvx1-symlink-ttyMODEM.rules ${ETCUDEVRULES}/40-rename-ttyrs485.rules "
		WORK=/tmp/_tmpfs.$$
		mkdir -p ${WORK}/usr/sbin
		mkdir -p ${WORK}/etc/init.d
		mkdir -p ${WORK}/etc/udev/rules.d
		cp -f ${OBSTOOLS} ${WORK}/usr/sbin
		cp -f ${OBSSCRIPTS} ${WORK}/etc/init.d
		cp -f ${OBSUDEVRULES} ${WORK}/etc/udev/rules.d
		(cd ${WORK}; tar cfzp ${RELEASEDIR}/obstools.tgz .)
		rm -rf ${WORK}
		;;
	*)
		;;
	esac

	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	case $DIST in
	stretch)
		(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs-stretch.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/obstools.tgz ${RELEASEDIR}/System.map)
		;;
	*)
		(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/System.map)
		;;
	esac
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsa16|obsfx0|obsfx1)
	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}
#	${OBJCOPY} -O binary -R .comment -S ${LINUX_SRC}/vmlinux ${RELEASEDIR}/${MAKE_IMAGE}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	# uboot env update script
	cp -f ${FILESDIR}/update_ubootenv-${TARGET}-${DIST}.sh ${RELEASEDIR}/update_ubootenv.sh
	# Device tree file
	ALT_DTBFILE=${ALT_DTBFILE:-${DTBFILE}}
	USB_BOOT_DTBFILE=`echo ${DTBFILE} | sed -e 's/\.dtb/-usb-boot\.dtb/'`
	ALT_USB_BOOT_DTBFILE=``echo ${ALT_DTBFILE} | sed -e 's/\.dtb/-usb-boot\.dtb/'`
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/freescale/${DTBFILE} ${RELEASEDIR}/${ALT_DTBFILE}
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/freescale/${USB_BOOT_DTBFILE} ${RELEASEDIR}/${ALT_USB_BOOT_DTBFILE}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs-dtb.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/Image ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/${ALT_DTBFILE} ${RELEASEDIR}/${ALT_USB_BOOT_DTBFILE} ${RELEASEDIR})
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsa16r|obsfx0r|obsfx1r)
	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR} && find . -print0 | cpio -o -a0 ) | ${COMP} -${COMP_LVL:-3} ${COMPOPT} > ${RELEASEDIR}/${TARGET}-rootfs.cpio.${COMPEXT}
	umount ${MOUNTDIR}

	# Create .its
	ALT_DTBFILE=${ALT_DTBFILE:-${DTBFILE}}
	LOAD_ADDRESS=${LOAD_ADDRESS:-"0x80000000"}
	ENTRY_ADDRESS=${ENTRY_ADDRESS:-${LOAD_ADDRESS}}
	sed -e 's|@COMPEXT@|'${COMPEXT}'|' \
	    -e 's|@ARCH@|'${ARCH}'|' \
	    -e 's|@COMP@|'${COMP}'|' \
	    -e 's|@LOAD_ADDRESS@|'${LOAD_ADDRESS}'|' \
	    -e 's|@ENTRY_ADDRESS@|'${ENTRY_ADDRESS}'|' \
	    -e 's|@DTBFILE@|'${ALT_DTBFILE}'|' \
	    -e 's|@TARGET@|'${TARGET}'|' \
		${FILESDIR}/uImage.its.in > ${RELEASEDIR}/${TARGET}-uImage.its

	# Ramdisk Image
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/freescale/${DTBFILE} ${RELEASEDIR}/${ALT_DTBFILE}
	(cd ${RELEASEDIR} && mkimage -f ${TARGET}-uImage.its uImage.initrd.${TARGET})

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})

	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh \
	${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/uImage.initrd.${TARGET} \
	${RELEASEDIR}/uImage.initrd.${TARGET} ${DIST} \
	${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} ${FILESDIR})

	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obstb3n)
	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	# Make Images
	(cd ${LINUX_SRC} && ./scripts/mkimg --dtb ${DTBFILE})
	cp -f ${LINUX_SRC}/boot.img ${RELEASEDIR}
	cp -f ${LINUX_SRC}/resource.img ${RELEASEDIR}
	cp -f ${LINUX_SRC}/zboot.img ${RELEASEDIR}

	# uboot env update script
	cp -f ${FILESDIR}/update_ubootenv-${TARGET}-${DIST}.sh ${RELEASEDIR}/update_ubootenv.sh
	# Device tree file
	ALT_DTBFILE=${ALT_DTBFILE:-${DTBFILE}}
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/rockchip/${DTBFILE} ${RELEASEDIR}/${ALT_DTBFILE}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs-dtb.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/Image ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/${ALT_DTBFILE} ${RELEASEDIR})
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obstb3nr)
	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	# Make Images
	(cd ${LINUX_SRC} && ./scripts/mkimg --dtb ${DTBFILE})
	cp -f ${LINUX_SRC}/boot.img ${RELEASEDIR}
	cp -f ${LINUX_SRC}/resource.img ${RELEASEDIR}
	cp -f ${LINUX_SRC}/zboot.img ${RELEASEDIR}

	# uboot env update script
	cp -f ${FILESDIR}/update_ubootenv-${TARGET}-${DIST}.sh ${RELEASEDIR}/update_ubootenv.sh
	# Device tree file
	ALT_DTBFILE=${ALT_DTBFILE:-${DTBFILE}}
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/rockchip/${DTBFILE} ${RELEASEDIR}/${ALT_DTBFILE}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs-dtb.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/Image ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/${ALT_DTBFILE} ${RELEASEDIR})
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsbx1)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}
		${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMPEXT}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})

	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh \
	${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage \
	${RELEASEDIR}/initrd.${COMPEXT} ${FILESDIR}/grub/grub.cfg \
	${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} ${FILESDIR})

	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
;;
obsbx1s)
	case $DIST in
	stretch)
		# obs tools
		USRSBIN=${DISTDIR}/usr/sbin
		USRLOCALSBIN=${DISTDIR}/usr/local/sbin
		OBSTOOLS="${USRSBIN}/wd-keepalive ${USRSBIN}/obs-util ${USRSBIN}/kosanu ${USRSBIN}/runled ${USRSBIN}/pshd ${USRSBIN}/atcmd ${USRSBIN}/obs-hwclock ${USRSBIN}/wav-play ${USRSBIN}/obsiot-power ${USRSBIN}/obsiot-modem.sh ${USRSBIN}/obsiot-power.sh ${USRSBIN}/flashcfg"
		OBSTOOLSLOCAL="${USRLOCALSBIN}/hwclock"
		ETCINITD=${DISTDIR}/etc/init.d
		OBSSCRIPTS="${ETCINITD}/obsiot-power ${ETCINITD}/nitz ${ETCINITD}/disable-modem"
		ETCUDEVRULES=${DISTDIR}/etc/udev/rules.d
		OBSUDEVRULES="${ETCUDEVRULES}/50-obsbx1-symlink-ttyMODEM.rules ${ETCUDEVRULES}/40-rename-ttyrs485.rules "
		WORK=/tmp/_tmpfs.$$
		mkdir -p ${WORK}/usr/sbin
		mkdir -p ${WORK}/usr/local/sbin
		mkdir -p ${WORK}/etc/init.d
		mkdir -p ${WORK}/etc/udev/rules.d
		cp -f ${OBSTOOLS} ${WORK}/usr/sbin
		cp -f ${OBSTOOLSLOCAL} ${WORK}/usr/local/sbin
		cp -f ${OBSSCRIPTS} ${WORK}/etc/init.d
		cp -f ${OBSUDEVRULES} ${WORK}/etc/udev/rules.d
		(cd ${WORK}; tar cfzp ${RELEASEDIR}/obstools.tgz .)
		rm -rf ${WORK}
		;;
	*)
		;;
	esac

	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	# uboot env update script
	cp -f ${FILESDIR}/update_ubootenv-${DIST}.sh ${RELEASEDIR}/update_ubootenv.sh

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	case $DIST in
	stretch)
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs-stretch.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/obstools.tgz ${RELEASEDIR}/System.map)
		;;
	*)
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/System.map)
		;;
	esac
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsix9|obshx1|obshx2)
	# Linux kernel
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}

	# Debian rootfs
	mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}
	(cd ${MOUNTDIR}; tar cfzp ${RELEASEDIR}/${TARGET}-rootfs.tgz .)
	umount ${MOUNTDIR}

	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-rootfs.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${FILESDIR}/flashcfg-rootfs.sh ${RELEASEDIR}/MD5.${TARGET} ${RELEASEDIR}/modules.tgz ${RELEASEDIR}/System.map)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
obsix9r|obshx1r|obshx2r)
	cp -f ${LINUX_SRC}/arch/${KERN_ARCH}/boot/bzImage ${RELEASEDIR}
	${COMP} -${COMP_LVL:-3} < ${_RAMDISK_IMG} > ${RELEASEDIR}/initrd.${COMPEXT}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	(cd ${WRKDIR}/build_ramdisk/kernel-image; ./mkdeb-obsiot.sh ${VERSION} ${ARCH} ${TARGET} ${RELEASEDIR}/bzImage ${RELEASEDIR}/initrd.${COMPEXT} dummy ${FILESDIR}/flashcfg.sh ${RELEASEDIR}/MD5.${TARGET} dummy)
	cp -f ${DISTDIR}/etc/openblocks-release ${RELEASEDIR}
	(cd ${RELEASEDIR}; rm -f MD5.${TARGET}; md5sum * > MD5.${TARGET})
	;;
esac
