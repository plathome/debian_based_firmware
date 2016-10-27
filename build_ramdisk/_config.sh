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

LANG=C
LANGUAGE=C
LC_ALL=C

WRKDIR=$(cd $(dirname $0)/..; pwd)

MOUNTDIR=${WRKDIR}/mnt
TMPDIR=${WRKDIR}/tmp

DISTDIR=${WRKDIR}/rootfs/${DIST}_${TARGET}

FILESDIR=${PWD}/files

ISOFILEDIR=${WRKDIR}/isofiles
EXTRADEBDIR=${WRKDIR}/extradebs/${DIST}

ETCDIR=${PWD}/etc.${DIST}
case $TARGET in
bpv4*)
	RELEASEDIR=${WRKDIR}/release/${TARGET}/${DIST}/${KERNEL}-${PATCHLEVEL}
	ETCDIR_ADD=${PWD}/etc.${DIST}.bpv4
	LINUX_SRC=${WRKDIR}/source/bpv4/linux-${KERNEL}
	RAMDISK_IMG=ramdisk-wheezy.${TARGET}.img
;;
*)
	RELEASEDIR=${WRKDIR}/release/${TARGET}/${DIST}/${KERNEL}-${PATCHLEVEL}
	ETCDIR_ADD=${PWD}/etc.${DIST}.${TARGET}
	LINUX_SRC=${WRKDIR}/source/${TARGET}/linux-${KERNEL}
	RAMDISK_IMG=ramdisk-${DIST}.${TARGET}.img
;;
esac

DEFCONFIGDIR=${PWD}/defconfigs

UBOOTDIR=${WRKDIR}/uboot-image/${TARGET}

case $ARCH in
amd64)
	CROSS_COMPILE=
	CC=gcc
	STRIP=strip
	KERN_ARCH=x86_64
	MAKE_IMAGE=bzImage
	QEMU_BIN=qemu-x86_64-static
	ABI=""
;;
i386)
	CROSS_COMPILE=
	CC=gcc
	STRIP=strip
	KERN_ARCH=x86
	MAKE_IMAGE=bzImage
	QEMU_BIN=qemu-i386-static
	ABI=""
;;
armhf)
	CROSS_COMPILE=arm-linux-gnueabihf-
	CC=${CROSS_COMPILE}gcc
	STRIP=${CROSS_COMPILE}strip
	KERN_ARCH=arm
	MAKE_IMAGE=zImage
	QEMU_BIN=qemu-arm-static
	ABI=eabi
;;
armel)
	CROSS_COMPILE=arm-linux-gnueabi-
	CC=${CROSS_COMPILE}gcc
	STRIP=${CROSS_COMPILE}strip
	KERN_ARCH=arm
	MAKE_IMAGE=zImage
	QEMU_BIN=qemu-arm-static
	ABI=eabi
;;
powerpc)
	CROSS_COMPILE=powerpc-linux-gnu-
	CC=${CROSS_COMPILE}gcc
	STRIP=${CROSS_COMPILE}strip
	KERN_ARCH=powerpc
	MAKE_IMAGE=uImage
	QEMU_BIN=qemu-ppc-static
	ABI=""
;;
*)
	CC=gcc
	STRIP=strip
;;
esac

MAKE_OPTION="ARCH=${KERN_ARCH} CROSS_COMPILE=${CROSS_COMPILE}"

if [ "$(uname -m)" == "x86_64" ] || [ "$(uname -m)" == "i686" ]; then
        CROSS=true
fi

if [ "$NOROOT" != "yes" ] && [ "$(id -u)" -ne "0" ]; then
	echo
	echo "ERROR: Please execute by root user."
	echo "ex) sudo $(basename $0)"
	echo
	exit 1
fi

trap 'echo;echo "arch: ${ARCH}, dist: ${DIST}, target: ${TARGET}"' EXIT
