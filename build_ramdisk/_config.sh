LANG=C
LANGUAGE=C
LC_ALL=C

WRKDIR=$(cd $(dirname $0)/..; pwd)

MOUNTDIR=${WRKDIR}/mnt

DISTDIR=${WRKDIR}/rootfs/${DIST}_${TARGET}

RELEASEDIR=${WRKDIR}/release/${TARGET}/${DIST}/${KERNEL}-${PATCHLEVEL}

FILESDIR=${PWD}/files

ISOFILEDIR=${WRKDIR}/isofiles
EXTRADEBDIR=${WRKDIR}/extradebs/${DIST}

ETCDIR=${PWD}/etc.${DIST}
ETCDIR_ADD=${PWD}/etc.${DIST}.${TARGET}

RAMDISK_IMG=ramdisk-${DIST}.${TARGET}.img

LINUX_SRC=${WRKDIR}/source/${TARGET}/linux-${KERNEL}
#LINUX_INC=${LINUX_SRC}/include



case $ARCH in
armel|armhf)
	CROSS_COMPILE=arm-linux-gnueabi-
	CC=${CROSS_COMPILE}gcc
	STRIP=${CROSS_COMPILE}strip
	KERN_ARCH=arm
;;
powrpc)
	CROSS_COMPILE=powerpc-linux-gnueabi-
	CC=${CROSS_COMPILE}gcc
	STRIP=${CROSS_COMPILE}strip
	KERN_ARCH=powerpc
 ;;
*)
	CC=gcc
	STRIP=strip
;;
esac


if [ "$(uname -m)" == "x86_64" ] || [ "$(uname -m)" == "i686" ]; then
        CROSS=true
fi

if [ "$(id -u)" -ne "0" ]; then
	echo
	echo "ERROR: Please execute by root user."
	echo "ex) sudo $(basename $0)"
	echo
	exit 1
fi

trap 'echo;echo "arch: ${ARCH}, dist: ${DIST}, target: ${TARGET}"' EXIT
