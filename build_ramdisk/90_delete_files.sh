#!/bin/bash
#
# Copyright (c) 2013 Plat'Home CO., LTD.
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

cd ${DISTDIR}

find usr/share/locale/ -maxdepth 1 | grep -Ev "(locale/|locale|ja)$" | xargs rm -rf

rm -rf var/lib/apt/lists/{ftp,security}* var/cache/apt/*.bin var/cache/apt/archives/*.deb root/.bash_history

if [ -f "${DISTDIR}/usr/share/doc/base-files/README" ] ; then
	mv usr/share/doc usr/share/doc.orig
	mkdir usr/share/doc
	(cd ${DISTDIR}/usr/share/doc.orig; \
	find . -maxdepth 2 -type f  -name copyright | xargs tar cf - ) | \
	(cd ${DISTDIR}/usr/share/doc; tar xvf -)
	rm -rf ${DISTDIR}/usr/share/doc.orig
fi

rm -f etc/ssh/ssh_host*key etc/ssh/ssh_host*key.pub

> etc/resolv.conf
