#!/bin/bash

. `dirname $0`/config.sh

#debug=echo

REPO=http://ftp.us.debian.org/debian

# disabled
#ISOFILE=
if [ -n "$ISOFILE" ] ;then
	if [ ! -f "${ISOFILEDIR}/${ISOFILE}" ]; then
		echo
		echo "$ISOFILE is not found."
		echo
		exit 1
	fi
	_REPO=/media/${DIST}-${ARCH}
	REPO=file://${_REPO}/debian
	$debug umount ${_REPO} 2> /dev/null
	$debug mkdir -p ${_REPO}
	$debug mount -o loop ${ISOFILEDIR}/${ISOFILE} ${_REPO}
fi

rm -rf   ${DISTDIR}
mkdir -p ${DISTDIR}

EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano"
#INCLUDE="gnu-fdisk,udev,openssh-server,lzma,strace"
INCLUDE="udev,openssh-server,lzma,strace"

$debug debootstrap --arch=${ARCH} \
	--exclude=${EXCLUDE} --include=${INCLUDE} ${DIST} ${DISTDIR} ${REPO}


$debug rm -f  ${DISTDIR}/etc/udev/rules.d/70-persistent-net.rules

$debug rm -rf ${DISTDIR}/dev/.udev

$debug mkdir -p ${DISTDIR}/usr/src/firmware

$debug umount ${_REPO} 2> /dev/null
