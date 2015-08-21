#
# Copyright (c) 2013-2015 Plat'Home CO., LTD.
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

DIST=${DIST:=wheezy}

TARGET=${TARGET:=obsax3}

COMPRESS=${COMPRESS:=lzma}

COMPRESS_EXT=${COMPRESS_EXT:=lzma}

export TARGET DIST

case ${DIST} in
wheezy)
	KERNEL=3.2.54
	case ${TARGET} in
	obsax3)
		DTBFILE=armada-xp-openblocks-ax3-4.dtb
		ISOFILE=debian-7.7.0-armhf-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=4
		ARCH=armhf
		UBOOT_VER=3.0.0
		UBOOT_PL=0
		UBOOT_OLD_VER="1\\\.0\\\.0\|1\\\.1\\\.[1235]\|2\\\.0\\\.[347]"
	;;
	obsa6)
		DTBFILE=kirkwood-openblocks_a6.dtb
		ISOFILE=debian-7.7.0-armel-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=2
		ARCH=armel
		LZMA_LEVEL=9
		UBOOT_VER=1.2.6
		UBOOT_PL=1
		UBOOT_OLD_VER="1\\\.0\\\.2\|1\\\.1\\\.0\|1\\\.2\\\.[34]"
	;;
	obsa7)
		DTBFILE=kirkwood-openblocks_a7.dtb
		ISOFILE=debian-7.7.0-armel-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=2
		ARCH=armel
		LZMA_LEVEL=9
		COMPRESS=gzip
		COMPRESS_EXT=gz
		UBOOT_VER=1.2.13
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.2\\\.[4689]\|1\\\.2\\\.1[2]"
	;;
	obs600)
		KERNEL=3.10.25
		DTBFILE=obs600.dtb
		ISOFILE=debian-7.4.0-powerpc-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=0
		ARCH=powerpc
		LZMA_LEVEL=9
		COMPRESS=gzip
		COMPRESS_EXT=gz
	;;
	*) exit 1 ;;
	esac
;;
jessie)
	KERNEL=4.0.5
	case ${TARGET} in
	obsax3)
		DTBFILE=armada-xp-openblocks-ax3-4.dtb
		ISOFILE=debian-8.1.0-armhf-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=beta0
		ARCH=armhf
		UBOOT_VER=3.0.0
		UBOOT_PL=0
		UBOOT_OLD_VER="1\\\.0\\\.0\|1\\\.1\\\.[1235]\|2\\\.0\\\.[347]"
	;;
	*) exit 1 ;;
	esac
;;
squeeze)
#	ISOFILE=debian-6.0.5-armel-DVD-1.iso
	ARCH=armel
	case ${TARGET} in
	obsax3)
		RAMDISK_SIZE=128
		KERNEL=3.0.6
		# 2013/06/17
		PATCHLEVEL=14
	;;
	obsa6)
		RAMDISK_SIZE=128
		KERNEL=2.6.31
		# 2013/01/31
		PATCHLEVEL=8
		LZMA_LEVEL=9
	;;
	obsa7)
		RAMDISK_SIZE=144
		KERNEL=3.2.40
		PATCHLEVEL=0beta0
		LZMA_LEVEL=9
		COMPRESS=gzip
		COMPRESS_EXT=gz
	;;
	*) exit 1 ;;
	esac
;;
esac

if [ -f _config.sh ] ; then
	. _config.sh
elif [ -f ../_config.sh ] ; then
	. ../_config.sh
else
	echo "could't read _config.sh"
	exit 1
fi
