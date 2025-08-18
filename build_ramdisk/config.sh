#
# Copyright (c) 2013-2025 Plat'Home CO., LTD.
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
ENA_SYSVINIT=false

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
		PATCHLEVEL=18
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
		COMPEXT=lzma
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
		COMPEXT=lzma
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
		COMPEXT=gz
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
		COMPEXT=gz
	;;
	*) exit 1 ;;
	esac
;;
jessie)
	KERNEL=4.1.25
	case ${TARGET} in
	obsmv4)
		ISOFILE=debian-8.4.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=1024
		DEFCONFIG=bpv4_defconfig
	;;
	obsvx1)
		KERNEL=4.4.112
		ISOFILE=debian-8.5.0-amd64-DVD-1.iso
		PATCHLEVEL=2
		RAMDISK_SIZE=230
#		ENA_VIRT=true
		DEFCONFIG=obsvx1_defconfig
	;;
	obsbx1)
		KERNEL=3.10.17
		ARCH=i386
		ISOFILE=debian-8.3.0-i386-DVD-1.iso
		PATCHLEVEL=104
		RAMDISK_SIZE=250
	;;
	obsax3)
		DTBFILE=armada-xp-openblocks-ax3-4.dtb
		ISOFILE=debian-8.5.0-armhf-DVD-1.iso
		RAMDISK_SIZE=180
		PATCHLEVEL=1
		ARCH=armhf
		COMP_LVL=9
		COMP=lzma
		COMPEXT=lzma
		UBOOT_VER=3.0.1
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.0\\\.0\|1\\\.1\\\.[1235]\|2\\\.0\\\.[347]\|3\\\.0\\\.0"
	;;
	obsa7)
		DTBFILE=kirkwood-openblocks_a7.dtb
		ISOFILE=debian-8.5.0-armel-DVD-1.iso
		RAMDISK_SIZE=180
		PATCHLEVEL=0
		ARCH=armel
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
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
#		COMPEXT=lzma
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
#		COMPEXT=gz
#		UBOOT_VER=1.2.13
#		UBOOT_PL=2
#		UBOOT_OLD_VER="1\\\.2\\\.[4689]\|1\\\.2\\\.1[2]"
#	;;
	*) exit 1 ;;
	esac
;;
stretch)
	KERNEL=4.9.168
	case ${TARGET} in
	obsvx1)
		ISOFILE=debian-9.5.0-amd64-DVD-1.iso
		PATCHLEVEL=0
		RAMDISK_SIZE=220
	;;
	obsvx2)
		KERNEL=4.9.228
		ISOFILE=debian-9.5.0-amd64-DVD-1.iso
		PATCHLEVEL=0
		RAMDISK_SIZE=200
	;;
	obsbx1s)
		KERNEL=3.10.98
		ARCH=i386
		ISOFILE=debian-9.5.0-i386-DVD-1.iso
		PATCHLEVEL=202
		RAMDISK_SIZE=250
		DEFCONFIG=obsbx1_defconfig
	;;
	obsax3)
		KERNEL=4.4.120
#		DTBFILE=armada-xp-openblocks-ax3-4.dtb
		DTBFILE=armada-xp-openblocks-ax3-2.dtb
		ISOFILE=debian-9.6.0-armhf-DVD-1.iso
		RAMDISK_SIZE=145
		PATCHLEVEL=0
		ARCH=armhf
		COMP_LVL=9
		COMP=lzma
		COMPEXT=lzma
#		DEFCONFIG=${TARGET}_defconfig
		DEFCONFIG=${TARGET}_sysvinit_defconfig
		ENA_SYSVINIT=true
		UBOOT_VER=3.0.1
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.0\\\.0\|1\\\.1\\\.[1235]\|2\\\.0\\\.[347]\|3\\\.0\\\.0"
	;;
	obsa7)
		DTBFILE=kirkwood-openblocks_a7.dtb
		ISOFILE=debian-9.6.0-armel-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=0
		ARCH=armel
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
#		DEFCONFIG=${TARGET}_sysvinit_defconfig
#		ENA_SYSVINIT=true
		UBOOT_VER=1.2.13
		UBOOT_PL=2
		UBOOT_OLD_VER="1\\\.2\\\.[4689]\|1\\\.2\\\.1[2]"
	;;
	obsbx1)
		KERNEL=3.10.17
		ARCH=i386
		ISOFILE=debian-9.5.0-i386-DVD-1.iso
		PATCHLEVEL=200
		RAMDISK_SIZE=210
	;;
	*)
	;;
	esac
;;
buster)
	KERNEL=4.19.85
	case ${TARGET} in
	obsvx1)
		KERNEL=4.19.198
		ISOFILE=debian-10.2.0-amd64-DVD-1.iso
		PATCHLEVEL=0
		RAMDISK_SIZE=260
	;;
	obsvx2)
		KERNEL=4.19.198
		ISOFILE=debian-10.2.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=260
	;;
	obsbx1s)
		KERNEL=3.10.98
		ISOFILE=debian-10.2.0-i386-DVD-1.iso
		ARCH=i386
		PATCHLEVEL=305
		RAMDISK_SIZE=240
	;;
	obsbx1)
		KERNEL=3.10.17
		ISOFILE=debian-10.2.0-i386-DVD-1.iso
		ARCH=i386
		PATCHLEVEL=302
		RAMDISK_SIZE=240
	;;
	obsix9)
		KERNEL=4.19.198
		ISOFILE=debian-10.2.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=512
	;;
	obsix9r)
		KERNEL=4.19.198
		ISOFILE=debian-10.2.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=512
	;;
	*)
	;;
	esac
;;
bullseye)
	KERNEL=5.10.176
	case ${TARGET} in
	obsvx2)
		ISOFILE=debian-11.1.0-amd64-DVD-1.iso
		PATCHLEVEL=3
#		DEFCONFIG=obsvx2_defconfig
		RAMDISK_SIZE=500
	;;
	obsvx1)
		ISOFILE=debian-11.1.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=280
	;;
	obsix9r)
		ISOFILE=debian-11.1.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=500
	;;
	obsix9)
		ISOFILE=debian-11.1.0-amd64-DVD-1.iso
		PATCHLEVEL=1
#		DEFCONFIG=obsvx2_defconfig
		RAMDISK_SIZE=500
	;;
	obshx1|obshx2)
		ISOFILE=debian-11.5.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=500
	;;
	obshx1r|obshx2r)
		ISOFILE=debian-11.5.0-amd64-DVD-1.iso
		PATCHLEVEL=1
		RAMDISK_SIZE=500
	;;
	obsbx1s)
		KERNEL=3.10.98
		ISOFILE=debian-11.1.0-i386-DVD-1.iso
		ARCH=i386
		PATCHLEVEL=401
		RAMDISK_SIZE=250
	;;
	obsbx1)
		KERNEL=3.10.17
		ISOFILE=debian-11.1.0-i386-DVD-1.iso
		ARCH=i386
		PATCHLEVEL=400
		RAMDISK_SIZE=250
	;;
	obsa16*)
		KERNEL=5.10.72
		ISOFILE=debian-11.1.0-arm64-DVD-1.iso
		RAMDISK_SIZE=260
		PATCHLEVEL=1
		DTBFILE=imx8mp-evk-obsa16.dtb
		ALT_DTBFILE=imx8mp-evk.dtb
		DEFCONFIG=obsa16_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
#		UBOOT_VER=2020.04
#		UDEFCONFIG=obsa16_defconfig
	;;
	obsfx0*)
		KERNEL=5.10.72
		ISOFILE=debian-11.1.0-arm64-DVD-1.iso
		RAMDISK_SIZE=230
		PATCHLEVEL=1
		DTBFILE=imx8mp-evk-obsfx0.dtb
		DEFCONFIG=obsfx0_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
		WIFI_DRIVER=summit-backports-12.103.0.5
#		WIFI_DRIVER=summit-backports-12.29.0.22
#		WIFI_DRIVER=laird-backport-11.171.0.24
		WIFI_DEFCONFIG=lwb
#		UBOOT_VER=2020.04
#		UDEFCONFIG=obsa16_defconfig
	;;
	obsfx1*)
		KERNEL=5.10.72
		ISOFILE=debian-11.1.0-arm64-DVD-1.iso
		RAMDISK_SIZE=260
		PATCHLEVEL=1
		DTBFILE=imx8mp-evk-obsfx1-vsc8531.dtb
#		DTBFILE=imx8mp-evk-obsfx1-rtl8211.dtb
		DEFCONFIG=obsfx1_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
		WIFI_DRIVER=summit-backports-12.103.0.5
#		WIFI_DRIVER=summit-backports-12.29.0.22
#		WIFI_DRIVER=laird-backport-11.171.0.24
		WIFI_DEFCONFIG=lwb
#		UBOOT_VER=2020.04
#		UDEFCONFIG=obsa16_defconfig
	;;
	obstb3n)
		KERNEL=5.10.160
		ISOFILE=debian-11.1.0-arm64-DVD-1.iso
		RAMDISK_SIZE=260
		PATCHLEVEL=0
		DTBFILE=rk3568-tinker_board_3n.dtb
		DEFCONFIG=tinker_board_3n_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
		COMPOPT=
	;;
	*)
	;;
	esac
;;
bookworm)
	KERNEL=6.1.123
	case ${TARGET} in
	obsvx1)
		PATCHLEVEL=1
		ISOFILE=debian-12.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=280
	;;
	obsvx2)
		PATCHLEVEL=1
		ISOFILE=debian-12.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obsix9)
		PATCHLEVEL=1
		ISOFILE=debian-12.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obsix9r)
		PATCHLEVEL=1
		ISOFILE=debian-12.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obshx1)
		PATCHLEVEL=1
		ISOFILE=debian-12.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obshx1r)
		PATCHLEVEL=1
		ISOFILE=debian-12.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obsa16*)
		KERNEL=6.1.55
		PATCHLEVEL=2
		ISOFILE=debian-12.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=300
		DTBFILE=imx8mp-evk-obsa16.dtb
		ALT_DTBFILE=imx8mp-evk.dtb
		DEFCONFIG=obsa16_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsfx0*)
		KERNEL=6.1.55
		PATCHLEVEL=4
		ISOFILE=debian-12.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=320
		DTBFILE=imx8mp-evk-obsfx0.dtb
		DEFCONFIG=obsfx0_defconfig
		WIFI_DRIVER=summit-backports-12.103.0.5
#		WIFI_DRIVER=summit-backports-12.29.0.22
#		WIFI_DRIVER=laird-backport-11.171.0.24
		WIFI_DEFCONFIG=lwb
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsfx1*)
		KERNEL=6.1.55
		PATCHLEVEL=4
		ISOFILE=debian-12.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=320
		DTBFILE=imx8mp-evk-obsfx1-vsc8531.dtb
#		DTBFILE=imx8mp-evk-obsfx1-rtl8211.dtb
		DEFCONFIG=obsfx1_defconfig
		WIFI_DRIVER=summit-backports-12.103.0.5
#		WIFI_DRIVER=summit-backports-12.29.0.22
#		WIFI_DRIVER=laird-backport-11.171.0.24
		WIFI_DEFCONFIG=lwb
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsgx4*)
		KERNEL=6.1.55
		PATCHLEVEL=2
		ISOFILE=debian-12.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=500
		DTBFILE=imx8mp-evk-obsgx4.dtb
		DEFCONFIG=obsgx4_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsduo*)
		KERNEL=6.1.55
		PATCHLEVEL=0
		ISOFILE=debian-12.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=360
		DTBFILE=imx8mp-evk-obsduo.dtb
		DEFCONFIG=obsduo_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	*)
	;;
	esac
;;
trixie)
	KERNEL=6.12.40
	case ${TARGET} in
	obsvx1)
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=300
	;;
	obsvx2)
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obsix9)
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obsix9r)
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obshx1)
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obshx1r)
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-amd64-DVD-1.iso
		RAMDISK_SIZE=500
	;;
	obsa16*)
		KERNEL=6.12.20
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=320
		DTBFILE=imx8mp-evk-obsa16.dtb
		ALT_DTBFILE=imx8mp-evk.dtb
		DEFCONFIG=obsa16_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsfx0*)
		KERNEL=6.12.20
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=320
		DTBFILE=imx8mp-evk-obsfx0.dtb
		DEFCONFIG=obsfx0_defconfig
		WIFI_DRIVER=summit-backports-13.24.0.20
		WIFI_DEFCONFIG=lwb
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsfx1*)
		KERNEL=6.12.20
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=320
		DTBFILE=imx8mp-evk-obsfx1-vsc8531.dtb
#		DTBFILE=imx8mp-evk-obsfx1-rtl8211.dtb
		DEFCONFIG=obsfx1_defconfig
		WIFI_DRIVER=summit-backports-13.24.0.20
		WIFI_DEFCONFIG=lwb
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	obsgx4*)
		KERNEL=6.12.20
		PATCHLEVEL=1
		ISOFILE=debian-13.0.0-arm64-DVD-1.iso
		RAMDISK_SIZE=500
		DTBFILE=imx8mp-evk-obsgx4.dtb
		DEFCONFIG=obsgx4_defconfig
		ARCH=arm64
		COMP_LVL=9
		COMP=gzip
		COMPEXT=gz
	;;
	*)
	;;
	esac
;;
esac

if [ -f local_config.sh ] ; then
	echo "read local_config.sh as it exists"
	. local_config.sh
fi

if [ -f _config.sh ] ; then
	. _config.sh
elif [ -f ../_config.sh ] ; then
	. ../_config.sh
else
	echo "could't read _config.sh"
	exit 1
fi
