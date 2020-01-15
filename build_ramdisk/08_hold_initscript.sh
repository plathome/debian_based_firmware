#!/bin/bash
#
# Copyright (c) 2013-2020 Plat'Home CO., LTD.
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
wheezy)
	chroot ${DISTDIR} /usr/bin/aptitude hold initscripts
	;;
jessie)
	cat > ${DISTDIR}/tmp/hold.$$ <<_HOLD2
	echo ifupdown hold | dpkg --set-selections
_HOLD2
	chroot ${DISTDIR} /bin/bash /tmp/hold.$$
	rm -f ${DISTDIR}/tmp/hold.$$
	;;
stretch)
	cat > ${DISTDIR}/tmp/hold.$$ <<_HOLD3
	echo kmod hold | dpkg --set-selections
	echo libkmod2 hold | dpkg --set-selections
	case $TARGET in
	obsbx1*)
		echo acpi-support-base hold | dpkg --set-selections ;;
	esac
_HOLD3
	chroot ${DISTDIR} /bin/bash /tmp/hold.$$
	rm -f ${DISTDIR}/tmp/hold.$$
	;;
esac

case $DIST in
squeeze|wheezy|jessie)
	cat > ${DISTDIR}/tmp/hold.$$ <<_HOLD
	echo initscripts hold | dpkg --set-selections
_HOLD
	chroot ${DISTDIR} /bin/bash /tmp/hold.$$
	rm -f ${DISTDIR}/tmp/hold.$$
	;;
*)	;;
esac
