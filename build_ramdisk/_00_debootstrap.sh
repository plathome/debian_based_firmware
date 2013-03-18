#!/bin/bash

. `dirname $0`/config.sh

rm -rf   ${DISTDIR}
mkdir -p ${DISTDIR}

debootstrap --arch=${ARCH} \
	--exclude=quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano \
	--include=gnu-fdisk,udev,openssh-server,lzma \
	${DIST} ${DISTDIR}

rm -f  ${DISTDIR}/etc/udev/rules.d/70-persistent-net.rules

rm -rf ${DISTDIR}/dev/.udev

mkdir -p ${DISTDIR}/usr/src/firmware
