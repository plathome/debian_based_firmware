#!/bin/bash

. `dirname $0`/config.sh

rm -rf   ${SQUEEZEDIR}
mkdir -p ${SQUEEZEDIR}

debootstrap --arch=${ARCH} \
	--exclude=quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano \
	--include=gnu-fdisk,udev,openssh-server,lzma \
	squeeze ${SQUEEZEDIR}

rm -f  ${SQUEEZEDIR}/etc/udev/rules.d/70-persistent-net.rules

rm -rf ${SQUEEZEDIR}/dev/.udev

mkdir -p ${SQUEEZEDIR}/usr/src/firmware
