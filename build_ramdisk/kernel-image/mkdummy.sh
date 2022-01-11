#!/bin/bash
#
# Copyright (c) 2013-2021 Plat'Home CO., LTD.
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

if [ "$#" -ne "3" ] ; then
	echo
	echo "usage: $0 [VERSION] [ARCH] [MODEL]"
	echo
	echo "ex) $0 1.0.0 armel obsa7"
	echo
	exit 1
fi

VERSION=$1
ARCH=$2
MODEL=$3

pkgdir=dummy-kernel-image-${VERSION}
case $MODEL in
obsa6|obsa7|obsax3)
	pkgstr="kernel-image"
	;;
*)
	pkgstr="kernel-image-$MODEL"
	;;
esac

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template;tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

mkdir -p $pkgdir/etc
echo $VERSION > $pkgdir/etc/openblocks-release
sed -e "s|__VERSION__|$VERSION|" \
    -e "s|__ARCH__|$ARCH|" \
    -e "s|__PACKAGE__|$pkgstr|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
case $MODEL in
obsax3|obsa6|obsa7)
	echo "Depends: uboot-image" >> /tmp/control.new
	;;
*)
	;;
esac
mv -f /tmp/control.new $pkgdir/DEBIAN/control

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

rm -rf $pkgdir

