#!/bin/bash

if [ "$#" -ne "2" ] ; then
	echo
	echo "usage: $0 [VERSION] [ARCH]"
	echo
	echo "ex) $0 1.0.0 armel"
	echo
	exit 1
fi

VERSION=$1
ARCH=$2
FIRM=$3

pkgdir=dummy-kernel-image-${VERSION}

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template;tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

mkdir -p $pkgdir/etc
echo $VERSION > $pkgdir/etc/openblocks-release
sed -e "s|__VERSION__|$VERSION|" \
    -e "s|__ARCH__|$ARCH|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

rm -rf $pkgdir

