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
	NOGPG="--no-check-gpg"
	_REPO=/media/${DIST}-${ARCH}
	REPO=file://${_REPO}/debian
	$debug umount ${_REPO} 2> /dev/null
	$debug mkdir -p ${_REPO}
	$debug mount -o loop ${ISOFILEDIR}/${ISOFILE} ${_REPO}
fi

if [ "$(uname -m)" == "x86_64" ] || [ "$(uname -m)" == "i686" ]; then
	CROSS=true
	FOREIGN="--foreign"
fi

rm -rf   ${DISTDIR}
mkdir -p ${DISTDIR}

EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano"
#INCLUDE="gnu-fdisk,udev,openssh-server,lzma,strace"
INCLUDE="udev,openssh-server,lzma,strace"

$debug debootstrap ${FOREIGN} ${NOGPG} --arch=${ARCH} \
	--exclude=${EXCLUDE} --include=${INCLUDE} ${DIST} ${DISTDIR} ${REPO}

if [ "$CROSS" == "true" ]; then
	# http://wiki.debian.org/EmDebian/CrossDebootstrap
	cp -fv $(which $QEMU_BIN) ${DISTDIR}/usr/bin/
	DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true \
	LC_ALL=C LANGUAGE=C LANG=C \
	chroot ${DISTDIR} /debootstrap/debootstrap --second-stage
	DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true \
	LC_ALL=C LANGUAGE=C LANG=C \
	chroot ${DISTDIR} dpkg --configure -a
fi


$debug rm -f  ${DISTDIR}/etc/udev/rules.d/70-persistent-net.rules

$debug rm -rf ${DISTDIR}/dev/.udev

$debug mkdir -p ${DISTDIR}/usr/src/firmware

$debug umount ${_REPO} 2> /dev/null
