#!/bin/bash

if [ "$#" -ne "1" ] ; then
	echo
	echo "usage: $0 [VERSION]"
	echo
	echo "ex) $0 1.0.0"
	echo
	exit 1
fi

VERSION=$1
FIRM=$2

pkgdir=dummy-kernel-image-${VERSION}

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template;tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

mkdir -p $pkgdir/etc
echo $VERSION > $pkgdir/etc/openblocks-release
sed -e "s|__VERSION__|$VERSION|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

rm -rf $pkgdir

