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

DIST=${DIST:=wheezy}

TARGET=${TARGET:=bpv4}

COMP=${COMP:=lzma}

COMPEXT=${COMPEXT:=lzma}

export TARGET DIST

ENA_BX1PM=false

KERNEL=3.12.35
ISOFILE=debian-7.8.0-amd64-DVD-1.iso
ARCH=amd64
COMP_LVL=3
COMP=gzip
COMPEXT=gz
DEFCONFIG=${TARGET}_defconfig
ENA_VIRT=false
ENA_AUDIO=false

case ${DIST} in
wheezy)
	case ${TARGET} in
	bpv4)
		PATCHLEVEL=1
		RAMDISK_SIZE=800
		ENA_VIRT=true
	;;
	bpv4-h)
		PATCHLEVEL=1
		RAMDISK_SIZE=200
		DEFCONFIG=bpv4_defconfig
	;;
	bpv8)
		PATCHLEVEL=1
		RAMDISK_SIZE=800
	;;
	obsbx1)
		KERNEL=3.10.17
		ARCH=i386
		ISOFILE=debian-7.8.0-i386-DVD-1.iso
		PATCHLEVEL=16
		RAMDISK_SIZE=200
		ENA_AUDIO=false
#		ENA_BX1PM=true
	;;
	obsax3)
		KERNEL=3.2.54
		ISOFILE=debian-7.7.0-armhf-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=5
		ARCH=armhf
		COMP_LVL=3
		COMP=lzma
		COMP_EXT=lzma
		UBOOT_VER=3.0.0
		UBOOT_PL=0
		UBOOT_OLD_VER="1\\\.0\\\.0\|1\\\.1\\\.[1235]\|2\\\.0\\\.[347]"
	;;
	obsa6)
		KERNEL=3.2.54
		ISOFILE=debian-7.7.0-armel-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=3
		ARCH=armel
		COMP_LVL=9
		COMP=lzma
		COMP_EXT=lzma
		UBOOT_VER=1.2.6
		UBOOT_PL=1
		UBOOT_OLD_VER="1\\\.0\\\.2\|1\\\.1\\\.0\|1\\\.2\\\.[34]"
	;;
	obsa7)
		KERNEL=3.2.54
		ISOFILE=debian-7.7.0-armel-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=3
		ARCH=armel
		COMP_LVL=9
		COMP=gzip
		COMP_EXT=gz
		UBOOT_VER=1.2.13
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.2\\\.[4689]\|1\\\.2\\\.1[2]"
	;;
	obs600)
		KERNEL=3.10.25
        DEFCONFIG=obs600_defconfig
		DTBFILE=obs600.dtb
		ISOFILE=debian-7.8.0-powerpc-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=0
		ARCH=powerpc
		COMP_LVL=9
		COMP=gzip
		COMP_EXT=gz
	;;
	*) exit 1 ;;
	esac
;;
jessie)
	KERNEL=4.1.19
	case ${TARGET} in
	obsmv4)
		ISOFILE=debian-8.4.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=1024
		DEFCONFIG=bpv4_defconfig
	;;
	obsbx1)
		KERNEL=3.10.17
		ARCH=i386
		ISOFILE=debian-8.3.0-i386-DVD-1.iso
		PATCHLEVEL=0
		RAMDISK_SIZE=250
	;;
	obsax3)
		DTBFILE=armada-xp-openblocks-ax3-4.dtb
		ISOFILE=debian-8.3.0-armhf-DVD-1.iso
		RAMDISK_SIZE=180
		PATCHLEVEL=2
		ARCH=armhf
		COMP_LVL=9
		COMP=lzma
		COMP_EXT=lzma
		UBOOT_VER=3.0.1
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.0\\\.0\|1\\\.1\\\.[1235]\|2\\\.0\\\.[347]\|3\\\.0\\\.0"
	;;
	obsa7)
		DTBFILE=kirkwood-openblocks_a7.dtb
		ISOFILE=debian-8.3.0-armel-DVD-1.iso
		RAMDISK_SIZE=180
		PATCHLEVEL=0
		ARCH=armel
		COMP_LVL=9
		COMP=gzip
		COMP_EXT=gz
		UBOOT_VER=1.2.13
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.2\\\.[4689]\|1\\\.2\\\.1[2]"
	;;
#	obsa6)
#		DTBFILE=kirkwood-openblocks_a6.dtb
#		ISOFILE=debian-8.3.0-armel-DVD-1.iso
#		RAMDISK_SIZE=180
#		PATCHLEVEL=0
#		ARCH=armel
#		COMP_LVL=9
#		COMP=lzma
#		COMP_EXT=lzma
#		UBOOT_VER=1.2.6
#		UBOOT_PL=1
#		UBOOT_OLD_VER="1\\\.0\\\.2\|1\\\.1\\\.0\|1\\\.2\\\.[34]"
#	;;
#	obs600)
#		DEFCONFIG=40x/obs600_defconfig
#		DTBFILE=obs600.dtb
#		ISOFILE=debian-8.3.0-powerpc-DVD-1.iso
#		RAMDISK_SIZE=210
#		PATCHLEVEL=0
#		ARCH=powerpc
#		COMP_LVL=9
#		COMP=gzip
#		COMP_EXT=gz
#		UBOOT_VER=1.2.13
#		UBOOT_PL=2
#		UBOOT_OLD_VER="1\\\.2\\\.[4689]\|1\\\.2\\\.1[2]"
#	;;
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
