#!/bin/bash
#
# Copyright (c) 2013-2020 Plat'Home CO., LTD.
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

LKSRC=${WRKDIR}/source/${TARGET}/dragonboard410c-LA.BR.1.2.7-03810-8x16.0-linaro2
CCPATH=/home/arm-eabi-4.8
SIGNLKPATH=/home/signlk

if [ ! -d "${LKSRC}" ]; then
	echo
	echo "lk source not exists."
	echo
	exit 1
elif [ ! -d "${CCPATH}" ]; then
	echo
	echo "cross compiler not exists."
	echo
	exit 1
elif [ ! -d "${SIGNLKPATH}" ]; then
	echo
	echo "sign lk not exists."
	echo
	exit 1
fi

[ "$TARGET" != "obsgem1" ] && exit 1
case $TARGET in
obsgem1)
	;;
*)
	echo
	echo "$TARGET is not supported."
	echo
	;;
esac
case $DIST in
squeeze|wheezy|jessie|stretch)
	echo
	echo "$DIST is not supported."
	echo
	exit 1
	;;
esac

cpunum=$(grep '^processor' /proc/cpuinfo  | wc -l)

cd ${LKSRC}
make -j$((${cpunum}+1)) ${TARGET} EMMC_BOOT=1 TOOLCHAIN_PREFIX=${CCPATH}/bin/arm-eabi-
mv ./build-${TARGET}/emmc_appsboot.mbn ./build-${TARGET}/emmc_appsboot_unsigned.mbn
${SIGNLKPATH}/signlk.sh -i=./build-${TARGET}/emmc_appsboot_unsigned.mbn -o=./build-${TARGET}/emmc_appsboot.mbn -d

cp ./build-${TARGET}/emmc_appsboot.mbn ${RELEASEDIR}
