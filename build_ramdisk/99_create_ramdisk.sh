#!/bin/bash
#
# Copyright (c) 2013-2016 Plat'Home CO., LTD.
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

size=${RAMDISK_SIZE:=160}

cd ${WRKDIR}

umount ${MOUNTDIR}

_RAMDISK_IMG=${DISTDIR}/../${RAMDISK_IMG}
rm -f ${_RAMDISK_IMG}

dd if=/dev/zero of=${_RAMDISK_IMG} count=0 seek=$(($size * 1024 * 1024 / 512))

echo y | mke2fs ${_RAMDISK_IMG}

mkdir -p ${MOUNTDIR}
mount -o loop ${_RAMDISK_IMG} ${MOUNTDIR}

(cd ${DISTDIR};tar --numeric-owner --exclude=${QEMU_BIN} -cpf - . | tar -xf - -C ${MOUNTDIR})

if [ "$ENA_VIRT" == "true" -a -f ${EXTRADEBDIR}/virtimg-${TARGET}-${DIST}.tar.xz ]; then
	tar -xJf ${EXTRADEBDIR}/virtimg-${TARGET}-${DIST}.tar.xz -C ${MOUNTDIR}
fi

case "$TARGET" in
bpv*)
	for f in "available" "available-old" "status" "status-old"; do
		sed -e "s|kernel-image-bpv.|kernel-image-$TARGET|" \
			< $MOUNTDIR/var/lib/dpkg/$f > /tmp/$f
		mv -f /tmp/$f $MOUNTDIR/var/lib/dpkg/$f
	done
	;;
*)
	;;
esac

umount ${MOUNTDIR}

tune2fs -c 0 -i 0 ${_RAMDISK_IMG}

