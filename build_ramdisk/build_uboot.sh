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

NOROOT=yes

. `dirname $0`/config.sh

case $TARGET in
obsgem*)
	UBOOT_SRC=${WRKDIR}/uboot-image/${TARGET}/u-boot-${UBOOT_VER}
	if [ ! -d "${UBOOT_SRC}" ]; then
		echo
		echo "U-boot source not exists."
		echo
		exit 1
	fi
	UMAKE_OPTION="ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}"
	;;
*)
	echo
	echo "$TARGET is not supported."
	echo
	exit 1
	;;
esac

cpunum=$(grep '^processor' /proc/cpuinfo  | wc -l)

cd ${UBOOT_SRC}

if [ ${UDEFCONFIG} ]; then
	make ${UMAKE_OPTION} ${UDEFCONFIG}
else
	make ${UMAKE_OPTION} ${TARGET}_defconfig
fi

if [ -f "${UBOOT_SRC}/../uboot-${UBOOT_VER}.dot.config" ]; then
	cp -f ${UBOOT_SRC}/../uboot-${UBOOT_VER}.dot.config .config
	make ${UMAKE_OPTION} oldconfig
fi

case $TARGET in
obsgem*)
	make -j$((${cpunum}+1)) ${UMAKE_OPTION}
	touch rd
	[ ! -d $RELEASEDIR ] && mkdir -p $RELEASEDIR
	${SKALESDIR}/dtbTool -o ${RELEASEDIR}/u-dt.img arch/arm/dts
	${SKALESDIR}/mkbootimg	--kernel u-boot-dtb.bin \
						--ramdisk rd \
						--output ${RELEASEDIR}/u-boot.img \
						--dt ${RELEASEDIR}/u-dt.img \
						--pagesize 2048 \
						--base 0x80000000 \
						--cmdline ""
	;;
esac

