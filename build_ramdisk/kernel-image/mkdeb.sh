#!/bin/bash

if [ "$#" -ne "3" ] ; then
	echo
	echo "usage: $0 [VERSION] [ARCH] [uImage.initrd]"
	echo
	echo "ex) $0 1.0.0-0 armel uImage.initrd.obsax3"
	echo
	exit 1
fi

VERSION=$1
ARCH=$2
FIRM=$3
FIRM_DIR=$(dirname $FIRM)

pkgdir=kernel-image-${VERSION}

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template;tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

echo $VERSION > $pkgdir/etc/openblocks-release
sed -e "s|__VERSION__|$VERSION|" \
    -e "s|__ARCH__|$ARCH|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

cp -vf $FIRM $pkgdir/etc/uImage.initrd

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

[ "$FIRM_DIR" != "." ] && mv -fv $pkgdir.deb $FIRM_DIR/

rm -rf $pkgdir

