#!/bin/bash

. `dirname $0`/config.sh

#debug=echo

rm -rf   ${DISTDIR}
mkdir -p ${DISTDIR}

REPO=http://ftp.us.debian.org/debian

# disabled
ISOFILE=
if [ -n "$ISOFILE" ] ;then
	_REPO=/media/${DIST}-${ARCH}
	REPO=file://${_REPO}/debian
	$debug umount ${_REPO} 2> /dev/null
	$debug mkdir -p ${_REPO}
	$debug mount -o loop ${ISOFILEDIR}/${ISOFILE} ${_REPO}
fi

$debug debootstrap --arch=${ARCH} \
	--exclude=quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano \
	--include=gnu-fdisk,udev,openssh-server,lzma,strace \
	${DIST} ${DISTDIR} ${REPO}


$debug rm -f  ${DISTDIR}/etc/udev/rules.d/70-persistent-net.rules

$debug rm -rf ${DISTDIR}/dev/.udev

$debug mkdir -p ${DISTDIR}/usr/src/firmware

$debug umount ${_REPO} 2> /dev/null
