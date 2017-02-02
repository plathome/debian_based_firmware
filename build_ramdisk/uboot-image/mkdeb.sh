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

if [ "$#" -ne "5" ] ; then
	echo
	echo "usage: $0 [VERSION] [ARCH] [TARGET] [obsa7-mtd0-1.2.13.img.xz] [OLD VERSION]"
	echo
	echo "ex) $0 1.2.13-0 armel obsa7 /path/obsa7-mtd0-1.2.13.img.xz \"1\.2\.[4689]|1\.2\.1[2]\""
	echo
	exit 1
fi

VERSION=$1
ARCH=$2
TARGET=$3
UBOOT=$4
OLD_VERSION=$5

pkgdir=uboot-image-$VERSION
UBOOTDIR=$(dirname $UBOOT)

rm -rf $pkgdir
mkdir -p $pkgdir
(cd template; tar -cf - .) | tar -xvf - -C $pkgdir/

sed -e "s|@VERSION@|$VERSION|" \
    -e "s|@ARCH@|$ARCH|" \
    < $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

NEW_VERSION=$(echo $VERSION | sed "s/-.*//g")
sed -e "s|@NEW_VERSION@|$NEW_VERSION|" \
    -e "s|@OLD_VERSION@|$OLD_VERSION|" \
    < $pkgdir/etc/init.d/uboot-image > /tmp/uboot-image.new
mv -f /tmp/uboot-image.new $pkgdir/etc/init.d/uboot-image
chmod 755 $pkgdir/etc/init.d/uboot-image

case ${TARGET} in
obsax3|obsa6)
	cp -f $UBOOTDIR/u-boot.${TARGET}.bin $pkgdir/etc/mtd0.data
;;
obsa7)
	cp -f $UBOOTDIR/obsa7-mtd0-${NEW_VERSION}.img.xz $pkgdir/etc/mtd0.data
	cp -f $UBOOTDIR/obsa7-mtd1-${NEW_VERSION}.img.xz $pkgdir/etc/mtd1.data
	cp -f $UBOOTDIR/obsa7-mtd1-${NEW_VERSION}-java.img.xz $pkgdir/etc/mtd1_java.data
;;
esac
cp -f $UBOOTDIR/tp.${TARGET}.bin $pkgdir/etc/mtd2.data

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

[ "$UBOOTDIR" != "." ] && mv -f $pkgdir.deb $UBOOTDIR/

rm -rf $pkgdir
