#!/bin/bash

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
make -j$((${cpunum}+1)) ARCH=${KERN_ARCH} CROSS_COMPILE=${CROSS_COMPILE} uImage modules
if [ "$TARGET" == "obs600" ]; then
	make ARCH=${KERN_ARCH} ${TARGET}.dtb
fi
