#!/bin/bash
#
# Copyright (c) 2013-2022 Plat'Home CO., LTD.
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
	if [ "$VERBOSE" == "yes" ]; then
		$debug umount ${_REPO}
	else
		$debug umount ${_REPO} 2> /dev/null
	fi
	$debug mkdir -p ${_REPO}
	$debug mount -o loop ${ISOFILEDIR}/${ISOFILE} ${_REPO}
fi

if [ "$CROSS" == "true" ]; then
	FOREIGN="--foreign"
fi

rm -rf   ${DISTDIR}
mkdir -p ${DISTDIR}

case $DIST in
wheezy|squeeze)
	case $TARGET in
	obsbx1)
		INCLUDE="openssh-server,strace,acpi-support-base"
		INCLUDE="$INCLUDE,wpasupplicant,python-gobject,ppp,wireless-tools,libnl-3-200,libnl-genl-3-200,libnl-route-3-200,ethtool,busybox,bluez,iw,libasound2"
		EXCLUDE="nano"
		;;
	obs600)
		EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano"
		INCLUDE="openssh-server,lzma,strace,perl"
		;;
	*)
		EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano"
		INCLUDE="openssh-server,strace"
		;;
	esac
	;;
jessie)
	case $TARGET in
	obsbx1)
		INCLUDE="openssh-server,strace,acpi-support-base"
		INCLUDE="$INCLUDE,wpasupplicant,ppp,wireless-tools,ethtool,busybox,bluez,iw,sysvinit,sysvinit-utils,dosfstools,libasound2,parted"
		EXCLUDE="nano"
		;;
	obsvx1)
		INCLUDE="openssh-server,strace,acpi-support-base"
		INCLUDE="$INCLUDE,wpasupplicant,ppp,wireless-tools,ethtool,bluez,iw,sysvinit,sysvinit-utils,dosfstools,libasound2,parted,bzip2,libusb-1.0-0"
		EXCLUDE="nano"
		;;
	obs600)
		EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,powerpc-utils,powerpc-ibm-utils,nano"
		INCLUDE="openssh-server,liblzo2-2,sysvinit,sysvinit-utils,parted,strace"
		;;
	obsa6)
		;;
	*)
		EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,nano"
		INCLUDE="openssh-server,lzma,parted,strace"
		;;
	esac
	;;
stretch)
	case $TARGET in
	obsvx*)
		INCLUDE="openssh-server,xz-utils,parted,insserv,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libnl-route-3-200"
		EXCLUDE="nano"
		;;
	obsbx*)
		INCLUDE="openssh-server,xz-utils,parted,insserv,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libnl-route-3-200,busybox"
		EXCLUDE="nano"
		;;
	obs600|obsa6)
		echo "Error: $TARGET is never supported."
		;;
	obsax3)
		INCLUDE="openssh-server,xz-utils,parted,insserv,busybox"
		EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,nano"
		;;
	*)
		INCLUDE="openssh-server,xz-utils,parted,insserv"
		EXCLUDE="quik,mac-fdisk,amiga-fdisk,hfsutils,yaboot,nano"
		;;
	esac
	;;
buster)
	case $TARGET in
	obsvx1)
		INCLUDE="openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget"
		EXCLUDE="nano"
		;;
	obsvx2)
		INCLUDE="openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget,apparmor"
		EXCLUDE="nano"
		;;
	obsbx1)
		INCLUDE="openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,busybox,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget"
		EXCLUDE="nano"
		;;
	obsbx1s)
		INCLUDE="openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,busybox,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget,apparmor"
		EXCLUDE="nano"
		;;
	obsix*)
		INCLUDE="openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,gnupg,strace,wget,apparmor,libiw30"
		EXCLUDE="nano"
		;;
	esac
	;;
bullseye)
	case $TARGET in
	obsvx1)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget"
		EXCLUDE="nano"
		;;
	obsvx2)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget,apparmor"
		EXCLUDE="nano"
		;;
	obsbx1)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,busybox,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget"
		EXCLUDE="nano"
		;;
	obsbx1s)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,wpasupplicant,ppp,wireless-tools,busybox,bluez,iw,libasound2,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,initscripts,gnupg,strace,wget,apparmor"
		EXCLUDE="nano"
		;;
	obsa16*)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libnl-route-3-200,initscripts,gnupg,strace,wget,apparmor"
		EXCLUDE="nano"
		;;
	obsfx1*)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,wpasupplicant,ppp,wireless-tools,bluez,iw,libasound2,libusb-1.0-0,libnl-route-3-200,initscripts,gnupg,strace,wget,apparmor"
		EXCLUDE="nano"
		;;
	obsix*)
		INCLUDE="iptables,openssh-server,xz-utils,parted,dosfstools,ethtool,acpi-support-base,libusb-1.0-0,libusb-0.1-4,libnl-route-3-200,gnupg,strace,wget,apparmor,libiw30"
		EXCLUDE="nano"
		;;
	esac
	;;
esac

if [ "$ENA_AUDIO" == "true" ]; then
	INCLUDE="$INCLUDE,alsa-utils"
fi

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

$debug rm -f ${DISTDIR}/etc/init.d/README ${DISTDIR}/etc/rc*/README

$debug mkdir -p ${DISTDIR}/usr/src/firmware

if [ -n "$ISOFILE" ]; then
	if [ "$VERBOSE" == "yes" ]; then
		$debug umount ${_REPO}
	else
		$debug umount ${_REPO} 2> /dev/null
	fi
fi
