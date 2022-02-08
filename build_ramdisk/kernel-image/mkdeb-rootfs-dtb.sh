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

#set -x

if [ "$#" -ne "8" ] ; then
	echo
	echo "usage: $0 [VERSION] [ARCH] [MODEL] [bzImage] [flashcfg] [MD5] [modules] [System.map] [DTB file] [uboot env file]"
	echo
	echo "ex) $0 1.0.0-0 arm64 obsa16 Image flashcfg MD5.obsa16 modules.tgz System.map dtbfile update_ubootenv.sh"
	echo
	exit 1
fi

VERSION=$1
ARCH=$2
MODEL=$3
FIRM=$4
FLASHCFG=$5
MD5=$6
DTB=$7
RELDIR=$8
FIRM_DIR=$(dirname $FIRM)

if [ "$MODEL" == "obsa16" -o "$MODEL" == "obsfx1" ]; then
	DESCRIPTION="Linux firmware for OpenBlocks A16"
	TARGET=$MODEL
else
	echo
	echo "$MODEL is not supported."
	echo
	exit 1
fi

pkgdir=kernel-image-${VERSION}-${TARGET}

rm -rf  $pkgdir
mkdir -p $pkgdir
(cd template;tar --exclude=CVS -cf - .) | tar -xvf - -C $pkgdir/

echo $VERSION > $pkgdir/etc/openblocks-release
sed -e "s|__VERSION__|$VERSION|" \
    -e "s|__ARCH__|$ARCH|" \
    -e "s|__PACKAGE__|kernel-image-$MODEL|" \
    -e "s|__DESCRIPTION__|$DESCRIPTION|" \
	< $pkgdir/DEBIAN/control > /tmp/control.new
mv -f /tmp/control.new $pkgdir/DEBIAN/control

cp -f $FIRM $pkgdir/etc/
cp -f $FLASHCFG $pkgdir/etc/flashcfg.sh
cp -f $MD5 $pkgdir/etc/
cp -f ${RELDIR}/modules.tgz $pkgdir/etc/
cp -f ${RELDIR}/System.map $pkgdir/etc/
cp -f $DTB $pkgdir/etc/
cp -f ${RELDIR}/update_ubootenv.sh $pkgdir/etc/

rm -rf ${pkgdir}.deb

dpkg-deb --build $pkgdir

[ "$FIRM_DIR" != "." ] && mv -fv $pkgdir.deb $FIRM_DIR/

rm -rf $pkgdir

