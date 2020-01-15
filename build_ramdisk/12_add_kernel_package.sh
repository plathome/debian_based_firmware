#!/bin/bash
#
# Copyright (c) 2013-2020 Plat'Home CO., LTD.
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

# dummy package: uboot-image
case $TARGET in
obsa*)
	(cd ${PWD}/uboot-image; rm -f dummy-uboot-image-0.0.0-0.deb)
	(cd ${PWD}/uboot-image; ./mkdummy.sh ${UBOOT_VER}-${UBOOT_PL} ${ARCH} ${TARGET} ${UBOOT_OLD_VER})
	cp -f ${PWD}/uboot-image/dummy-uboot-image-0.0.0-0.deb ${DISTDIR}/
	chroot ${DISTDIR} dpkg -r kernel-image
	chroot ${DISTDIR} dpkg -r uboot-image
	chroot ${DISTDIR} dpkg -i /dummy-uboot-image-0.0.0-0.deb
	rm -f ${DISTDIR}/dummy-uboot-image-0.0.0-0.deb
	rm -f ${PWD}/uboot-image/dummy-uboot-image-0.0.0-0.deb
	;;
*)
	;;
esac

# dummy package: kernel-image
(cd ${PWD}/kernel-image; rm -f dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb)
(cd ${PWD}/kernel-image; ./mkdummy.sh ${KERNEL}-${PATCHLEVEL} ${ARCH} ${TARGET})

cp -f ${PWD}/kernel-image/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb ${DISTDIR}/
chroot ${DISTDIR} dpkg -r kernel-image
chroot ${DISTDIR} dpkg -i /dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb
rm -f ${DISTDIR}/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb
rm -f ${PWD}/kernel-image/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb


if [ "$DTBFILE" == "armada-xp-openblocks-ax3-2.dtb" ]; then
	cat > ${DISTDIR}/tmp/hold.$$ <<_HOLD4
	echo kernel-image hold | dpkg --set-selections
_HOLD4
	chroot ${DISTDIR} /bin/bash /tmp/hold.$$
	rm -f ${DISTDIR}/tmp/hold.$$
fi
