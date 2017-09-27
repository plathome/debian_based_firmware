#!/bin/bash
#
# Copyright (c) 2013-2017 Plat'Home CO., LTD.
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

case ${TARGET} in
obs*)
	chmod 755 ${DISTDIR}/etc/init.d/openblocks-setup
	chroot ${DISTDIR} /sbin/insserv -rf openblocks-setup
	chroot ${DISTDIR} /sbin/insserv openblocks-setup
	chmod 755 ${DISTDIR}/etc/init.d/runled
	chroot ${DISTDIR} /sbin/insserv -rf runled
	chroot ${DISTDIR} /sbin/insserv runled
	chmod 755 ${DISTDIR}/etc/init.d/pshd
	chroot ${DISTDIR} /sbin/insserv -rf pshd
	chroot ${DISTDIR} /sbin/insserv pshd
	;;
*)
	;;
esac

case ${TARGET} in
obsbx1|obsvx1)
	chmod 755 ${DISTDIR}/etc/init.d/bluetooth
	chroot ${DISTDIR} /sbin/insserv -rf bluetooth
	chroot ${DISTDIR} /sbin/insserv bluetooth
	chmod 755 ${DISTDIR}/etc/init.d/nitz
	chroot ${DISTDIR} /sbin/insserv -rf nitz
	chroot ${DISTDIR} /sbin/insserv nitz
	chmod 755 ${DISTDIR}/etc/init.d/wd-keepalive
	chroot ${DISTDIR} /sbin/insserv -rf wd-keepalive
	chroot ${DISTDIR} /sbin/insserv wd-keepalive
	chmod 755 ${DISTDIR}/etc/init.d/disable-modem
	chroot ${DISTDIR} /sbin/insserv -rf disable-modem
	chroot ${DISTDIR} /sbin/insserv disable-modem
	if [ ${ENA_BX1PM} == "true" ]; then
		chmod 755 ${DISTDIR}/etc/init.d/enable-pm
		chroot ${DISTDIR} /sbin/insserv -rf enable-pm
		chroot ${DISTDIR} /sbin/insserv enable-pm
	fi
	if [ "$TARGET" == "obsvx1" ]; then
		chmod 755 ${DISTDIR}/etc/init.d/instfirm
		chroot ${DISTDIR} /sbin/insserv -rf instfirm
		chroot ${DISTDIR} /sbin/insserv instfirm
		chmod 755 ${DISTDIR}/etc/init.d/power-saving.sh
		chroot ${DISTDIR} /sbin/insserv -rf power-saving.sh
		chroot ${DISTDIR} /sbin/insserv power-saving.sh
	fi
	;;
*)
	;;
esac
chroot ${DISTDIR} /sbin/insserv

touch ${DISTDIR}/etc/init.d/.legacy-bootordering

if [ -f ${DISTDIR}/etc/modules ]; then
	if [ "$DIST" != "stretch" ]; then
		if grep -q "^ipv6" ${DISTDIR}/etc/modules; then
			echo "/etc/modules: The line, ipv6, exists"
		else
			echo "ipv6" >> ${DISTDIR}/etc/modules
		fi
	fi

	if [ "$TARGET" == "obsvx1" ];then
		if grep -q "^rtl8821ae" ${DISTDIR}/etc/modules; then
			echo "/etc/modules: The line, rtl8821ae, exists"
		else
			echo "rtl8821ae" >> ${DISTDIR}/etc/modules
		fi
		if grep -q "^ath10k_pci" ${DISTDIR}/etc/modules; then
			echo "/etc/modules: The line, ath10k_pci, exists"
		else
			echo "ath10k_pci" >> ${DISTDIR}/etc/modules
		fi
	fi

	if [ "$ENA_VIRT" == "true" ]; then
		if grep -q "^kvm-intel" ${DISTDIR}/etc/modules; then
			echo "/etc/modules: The line, kvm-intel, exists"
		else
			echo "kvm-intel" >> ${DISTDIR}/etc/modules
		fi
	fi
fi

if [ ${DIST} == "jessie" -o ${DIST} == "stretch" ]; then
	sed -e "s|^PermitRootLogin without-password|PermitRootLogin yes|" \
		< ${DITDIR}/etc/ssh/sshd_config > /tmp/sshd_config.new
	mv -f /tmp/sshd_config.new ${DISTDIR}/etc/ssh/sshd_config
fi

if [ ${DIST} == "stretch" ]; then
	if [ ! -e "${DISTDIR}/etc/udev/rules.d/73-usb-net-by-mac.rules" ]; then
		ln -s /dev/null ${DISTDIR}/etc/udev/rules.d/73-usb-net-by-mac.rules
	fi
	if [ ! -e "${DISTDIR}/etc/udev/rules.d/75-net-description.rules" ]; then
		ln -s /dev/null ${DISTDIR}/etc/udev/rules.d/75-net-description.rules
	fi
fi
