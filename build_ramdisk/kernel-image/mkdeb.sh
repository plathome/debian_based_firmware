#!/bin/bash

if [ "$#" -ne "2" ] ; then
	echo
	echo "usage: $0 [VERSION] [uImage.initrd]"
	echo
	echo "ex) $0 1.0.0 uImage.initrd.obsax3"
	echo
	exit 1
fi

VERSION=$1
FIRM=$2
FIRM_DIR=$(dirname $FIRM)

pkgdir=kernel-image-${VERSION}

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template;tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

echo $VERSION > $pkgdir/etc/openblocks-release
sed -e "s|__VERSION__|$VERSION|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

cp -vf $FIRM $pkgdir/etc/uImage.initrd

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

mv -fv $pkgdir.deb $FIRM_DIR/

rm -rf $pkgdir

