#!/bin/bash
#
# Copyright (c) 2013-2025 Plat'Home CO., LTD.
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

(cd ${ETCDIR};sort exclude.list > /tmp/exclude.list.new; mv -f /tmp/exclude.list.new exclude.list)

(cd ${ETCDIR};tar --exclude=CVS -cf - .)| tar xf - -C ${DISTDIR}/etc/
(cd ${ETCDIR_ADD};tar --exclude=CVS -cf - .)| tar xf - -C ${DISTDIR}/etc/

(cd ${DISTDIR}/etc; (cd ${ETCDIR};find . -type f) | xargs chown root:root ${DISTDIR}/etc)

(cd ${DISTDIR}/etc; (cd ${ETCDIR_ADD};find . -type f) | xargs chown root:root ${DISTDIR}/etc)

if [ "$DIST" == "stretch" ]; then
	case $TARGET in
	obsa*)
		for file in pshd runled;
		do
			sed -e "s|# Required-Start:    \$local_fs \$syslog \$remote_fs nitz|# Required-Start:    $local_fs $syslog $remote_fs|"	\
			< ${DISTDIR}/etc/init.d/${file} > /tmp/${file}.new
			mv -f /tmp/${file}.new ${DISTDIR}/etc/init.d/${file}
		done
	;;
	*)
	;;
	esac
fi

case ${TARGET} in
obsbx*|obsvx*)
	if [ "$DIST" != "buster" ] && [ "$DIST" != "bullseye" ] && [ "$DIST" != "bookworm" ] && [ "$DIST" != "trixie" ] ; then
		chmod 755 ${DISTDIR}/etc/init.d/bluetooth
		chroot ${DISTDIR} /sbin/insserv -rf bluetooth
		chroot ${DISTDIR} /sbin/insserv bluetooth
		chmod 755 ${DISTDIR}/etc/init.d/nitz
		chroot ${DISTDIR} /sbin/insserv -rf nitz
		chroot ${DISTDIR} /sbin/insserv nitz
		chmod 755 ${DISTDIR}/etc/init.d/obsiot-power
		chroot ${DISTDIR} /sbin/insserv -rf obsiot-power
		chroot ${DISTDIR} /sbin/insserv obsiot-power
		chmod 755 ${DISTDIR}/etc/init.d/openblocks-setup
		chroot ${DISTDIR} /sbin/insserv -rf openblocks-setup
		chroot ${DISTDIR} /sbin/insserv openblocks-setup
		chmod 755 ${DISTDIR}/etc/init.d/runled
		chroot ${DISTDIR} /sbin/insserv -rf runled
		chroot ${DISTDIR} /sbin/insserv runled
		chmod 755 ${DISTDIR}/etc/init.d/pshd
		chroot ${DISTDIR} /sbin/insserv -rf pshd
		chroot ${DISTDIR} /sbin/insserv pshd
		chmod 755 ${DISTDIR}/etc/init.d/wd-keepalive
		chroot ${DISTDIR} /sbin/insserv -rf wd-keepalive
		chroot ${DISTDIR} /sbin/insserv wd-keepalive

		case $TARGET in
		obsvx*)
			chmod 755 ${DISTDIR}/etc/init.d/instfirm
			chroot ${DISTDIR} /sbin/insserv -rf instfirm
			chroot ${DISTDIR} /sbin/insserv instfirm
			chmod 755 ${DISTDIR}/etc/init.d/disable-modem
			chroot ${DISTDIR} /sbin/insserv -rf disable-modem
			chroot ${DISTDIR} /sbin/insserv disable-modem
			;;
		obsbx1*)
			chmod 755 ${DISTDIR}/etc/init.d/reset-smsc95xx
			chroot ${DISTDIR} /sbin/insserv -rf reset-smsc95xx
#			chroot ${DISTDIR} /sbin/insserv reset-smsc95xx
			chmod 755 ${DISTDIR}/etc/init.d/disable-modem
			chroot ${DISTDIR} /sbin/insserv -rf disable-modem
			chroot ${DISTDIR} /sbin/insserv disable-modem
			;;
		esac
	else
		mount -t proc none ${DISTDIR}/proc
		mount -o bind /sys ${DISTDIR}/sys

		chroot ${DISTDIR} /usr/bin/systemctl enable acpid
		case $TARGET in
		obsvx1|obsbx1)
			;;
		*)
			chroot ${DISTDIR} /usr/bin/systemctl disable apparmor
			;;
		esac

		umount ${DISTDIR}/proc
		umount ${DISTDIR}/sys
	fi
	;;
obsa16*|obsfx0*|obsfx1*|obsgx4*|obsduo)
	mount -t proc none ${DISTDIR}/proc
	mount -o bind /sys ${DISTDIR}/sys

	chroot ${DISTDIR} /usr/bin/systemctl disable apparmor
	chroot ${DISTDIR} /usr/bin/systemctl disable nfs-server

	umount ${DISTDIR}/proc
	umount ${DISTDIR}/sys
	;;
obstb3n)
	mount -t proc none ${DISTDIR}/proc
	mount -o bind /sys ${DISTDIR}/sys

	chroot ${DISTDIR} /usr/bin/systemctl disable apparmor
	chroot ${DISTDIR} /usr/bin/systemctl disable nfs-server

	umount ${DISTDIR}/proc
	umount ${DISTDIR}/sys
	;;
obsix*|obshx1*|obshx2*)
	mount -t proc none ${DISTDIR}/proc
	mount -o bind /sys ${DISTDIR}/sys

	chroot ${DISTDIR} /usr/bin/systemctl disable apparmor
	chroot ${DISTDIR} /usr/bin/systemctl enable acpid

	umount ${DISTDIR}/proc
	umount ${DISTDIR}/sys
	;;
*)	;;
esac

[ -f /sbin/insserv ] && chroot ${DISTDIR} /sbin/insserv

touch ${DISTDIR}/etc/init.d/.legacy-bootordering

if [ -f ${DISTDIR}/etc/modules ]; then
	case $DIST in
	squeeze|wheezy|jessie)
		if grep -q "^ipv6" ${DISTDIR}/etc/modules; then
			echo "/etc/modules: The line, ipv6, exists"
		else
			echo "ipv6" >> ${DISTDIR}/etc/modules
		fi
		;;
	esac

	case $TARGET in
	obsvx*)
		if grep -q "^rtl8821ae" ${DISTDIR}/etc/modules; then
			echo "/etc/modules: The line, rtl8821ae, exists"
		else
			echo "rtl8821ae" >> ${DISTDIR}/etc/modules
		fi
#		if grep -q "^ath10k_pci" ${DISTDIR}/etc/modules; then
#			echo "/etc/modules: The line, ath10k_pci, exists"
#		else
#			echo "ath10k_pci" >> ${DISTDIR}/etc/modules
#		fi
		;;
	esac
fi

case $DIST in
stretch|buster|bullseye|bookworm|trixie)
	sed -e "s|^PermitRootLogin without-password|PermitRootLogin yes|" \
		-e "s|^#PermitRootLogin prohibit-password|PermitRootLogin yes|" \
		< ${DISTDIR}/etc/ssh/sshd_config > /tmp/sshd_config.new
	mv -f /tmp/sshd_config.new ${DISTDIR}/etc/ssh/sshd_config

	case $TARGET in
	obshx1*|obshx2*)
		;;
	*)
		if [ ! -e "${DISTDIR}/etc/udev/rules.d/73-usb-net-by-mac.rules" ]; then
			ln -s /dev/null ${DISTDIR}/etc/udev/rules.d/73-usb-net-by-mac.rules
		fi

		if [ ! -e "${DISTDIR}/etc/udev/rules.d/75-net-description.rules" ]; then
			ln -s /dev/null ${DISTDIR}/etc/udev/rules.d/75-net-description.rules
		fi
		;;
	esac
	;;
jessie)
	sed -e "s|^PermitRootLogin without-password|PermitRootLogin yes|" \
		-e "s|^#PermitRootLogin prohibit-password|PermitRootLogin yes|" \
		< ${DISTDIR}/etc/ssh/sshd_config > /tmp/sshd_config.new
	mv -f /tmp/sshd_config.new ${DISTDIR}/etc/ssh/sshd_config
	;;
*)	;;
esac
