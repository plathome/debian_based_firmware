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

ls ${EXTRADEBDIR}/*${ARCH}.deb 2> /dev/null
if [ $? -eq 0 ]; then
	rm -f ${DISTDIR}/*.deb
	rsync ${EXTRADEBDIR}/*${ARCH}.deb ${DISTDIR}/ ${EXCLUDE}
	rsync ${EXTRADEBDIR}/*all.deb ${DISTDIR}/ ${EXCLUDE}
	if [ "$ENA_SYSVINIT" == "true" ]; then
		rsync ${EXTRADEBDIR}/sysvinit/*${ARCH}.deb ${DISTDIR}/ ${EXCLUDE}
		rsync ${EXTRADEBDIR}/sysvinit/*all.deb ${DISTDIR}/ ${EXCLUDE}
	fi

	debs=$(cd ${DISTDIR}/; ls -1 *${ARCH}.deb 2> /dev/null)
	debs+=" $(cd ${DISTDIR}/; ls -1 *all.deb 2> /dev/null)"

	chroot ${DISTDIR} dpkg -i $debs

	rm -f ${DISTDIR}/*.deb
fi
