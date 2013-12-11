#!/bin/bash
#
# Copyright (c) 2013 Plat'Home CO., LTD.
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


if [ ! -d "${LINUX_SRC}" ]; then
	echo
	echo "Linux source not exists."
	echo
	exit 1
fi

cpunum=$(grep '^processor' /proc/cpuinfo  | wc -l)

if [ "$TARGET" == "obs600" ]; then
	if [ ! -h ${LINUX_SRC}/include/asm ]; then
		cd ${LINUX_SRC}/include
		ln -s ../arch/powerpc/include/asm asm-powerpc
		ln -s asm-powerpc asm
	fi
fi

cd ${LINUX_SRC}
make ARCH=${KERN_ARCH} CROSS_COMPILE=${CROSS_COMPILE} ${TARGET}_defconfig
make -j$((${cpunum}+1)) ARCH=${KERN_ARCH} CROSS_COMPILE=${CROSS_COMPILE} zImage modules
if [ "$TARGET" == "obs600" ]; then
	make ARCH=${KERN_ARCH} ${TARGET}.dtb
fi
