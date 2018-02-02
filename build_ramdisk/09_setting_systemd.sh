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

case $DIST in
jessie)
	echo ${DISTDIR}
	chroot ${DISTDIR} /usr/bin/apt-get remove --purge --auto-remove -y systemd
	echo -e 'Package: systemd\nPin: origin ""\nPin-Priority: -1' > ${DISTDIR}/etc/apt/preferences.d/systemd
	echo -e '\n\nPackage: *systemd*\nPin: origin ""\nPin-Priority: -1' >> ${DISTDIR}/etc/apt/preferences.d/systemd
	buf="\n\nPackage: systemd:$ARCH\nPin: origin ""\nPin-Priority: -1"
	echo -e $buf >> ${DISTDIR}/etc/apt/preferences.d/systemd
	;;
stretch)
	if [ "$ENA_SYSVINIT" == "true" ]; then
		chroot ${DISTDIR} /usr/bin/apt remove --purge --auto-remove -y systemd
		echo -e 'Package: *systemd*\nPin: release *\nPin-Priority: -1\n' > ${DISTDIR}/etc/apt/preferences.d/systemd
		(cd ${ETCDIR}.sysvinit;tar --exclude=CVS -cf - .) | tar xf - -C ${DISTDIR}/etc/
		chmod 755 ${DISTDIR}/etc/init.d/openblocks-setup
		chroot ${DISTDIR} /sbin/insserv -rf openblocks-setup
		chroot ${DISTDIR} /sbin/insserv openblocks-setup
	else
		if [ "$TARGET" == "obsvx2" ]; then
			cp -f ${ETCDIR}.sysvinit/init.d/openblocks-setup ${DISTDIR}/etc/init.d/
			chmod 755 ${DISTDIR}/etc/init.d/openblocks-setup
			chroot ${DISTDIR} /sbin/insserv -rf openblocks-setup
			chroot ${DISTDIR} /sbin/insserv openblocks-setup
		fi
		chroot ${DISTDIR} /bin/systemctl enable rc-local.service
	fi
	;;
*)
	;;
esac
