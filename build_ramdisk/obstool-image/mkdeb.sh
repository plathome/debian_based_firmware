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

if [ "$#" -ne "5" ] ; then
	echo
	echo "usage: $0 [VERSION] [ARCH] [PACKAGE] [RELEASE PATH] [DEPENDS]"
	echo
	echo "ex) $0 1 amd64 obsvx2 runled"
	echo
	exit 1
fi

VERSION=$1
ARCH=$2
PACKAGE=$3
REL_DIR=$4
DEPENDS=$5

pkgdir=${PACKAGE}-${VERSION}-${ARCH}

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template-${PACKAGE};tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

sed -e "s|__VERSION__|$VERSION|" \
    -e "s|__ARCH__|$ARCH|" \
    -e "s|__DEPENDS__|$DEPENDS|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

if [ -e "${pkgdir}/lib/systemd/system/${PACKAGE}.service" ] ; then
	case "$DIST" in
		"buster")
			sed -e "s|^KillMode=mixed|KillMode=none|" -i ${pkgdir}/lib/systemd/system/${PACKAGE}.service
			;;
		*)
			sed -e "s|^KillMode=none|KillMode=mixed|" -i ${pkgdir}/lib/systemd/system/${PACKAGE}.service
			;;
	esac
fi

rm -rf ${pkgdir}.deb

dpkg-deb --build --root-owner-group ${pkgdir}

mv -fv ${pkgdir}.deb ${REL_DIR}/

[ -e template-${PACKAGE}/usr ] && rm -rf template-${PACKAGE}/usr
[ -e template-atcmd/etc ] && rm -rf template-atcmd/etc
rm -rf ${pkgdir}

