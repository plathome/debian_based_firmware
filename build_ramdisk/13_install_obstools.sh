#!/bin/bash
#
# Copyright (c) 2013-2018 Plat'Home CO., LTD.
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
. `dirname $0`/_obstool_version.sh

case $TARGET in
obsvx2)
#	pkglist="atcmd flashcfg obs_util obs_hwclock obsiot_power pshd runled wd_keepalive"
	pkglist="obs_util pshd runled wd_keepalive"
	;;
*)
	exit 1 ;;
esac

apt-get -y install libi2c-dev

LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

case $TARGET in
obsvx*)
	CFLAGS="-Wall -I/usr/${KERN_ARCH}-linux-gnu${ABI}/include -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -li2c -O2 -mstackrealign -fno-omit-frame-pointer -DCONFIG_OBSVX1"
	;;
obsbx*)
	CFLAGS="-Wall -I/usr/include/${ARCH}-linux-gnu${ABI}/ -L/usr/lib/${ARCH}-linux-gnu${ABI}/ -L/lib/${ARCH}-linux-gnu${ABI}/ -m32 -O2 -march=core2 -mtune=core2 -msse3 -mfpmath=sse -mstackrealign -fno-omit-frame-pointer -DCONFIG_OBSBX1"
	;;
obsgem*)
	CFLAGS="-Wall -I/usr/include/${KERN_ARCH}-linux-gnu${ABI}/ -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -O2 -fno-omit-frame-pointer -DCONFIG_OBSGEM1"
	;;
*) exit 1 ;;
esac

echo "WD-KEEPALIVE"
$CC -o ${OBSTOOLDIR}/template-wd-keepalive/usr/sbin/wd-keepalive ${FILESDIR}/wd-keepalive.c $CFLAGS
echo "OBS-UTIL"
$CC -o ${OBSTOOLDIR}/template-obs-util/usr/sbin/obs-util ${FILESDIR}/obs-util.c $CFLAGS
$CC -o ${OBSTOOLDIR}/template-obs-util/usr/sbin/kosanu ${FILESDIR}/kosanu.c $CFLAGS
echo "RUNLED"
$CC -o ${OBSTOOLDIR}/template-runled/usr/sbin/runled ${FILESDIR}/runled_bx1.c $CFLAGS
echo "PSHD"
$CC -o ${OBSTOOLDIR}/template-pshd/usr/sbin/pshd ${FILESDIR}/pshd_bx1.c $CFLAGS

for pkg in $pkglist; do
	eval version='$'${pkg}_ver
	pkg=${pkg//_/-}
	pkgfile=${pkg}-${version}-${TARGET}.deb
	rm -f ${RELEASEDIR}/${pkgfile}
	(cd ${OBSTOOLDIR}/; ./mkdeb.sh ${version} ${ARCH} ${TARGET} ${pkg} ${RELEASEDIR} "")
	cp -f ${RELEASEDIR}/${pkgfile} ${DISTDIR}/
	chroot ${DISTDIR} dpkg -r ${pkg}
	chroot ${DISTDIR} dpkg -i ${pkgfile}
	rm -f ${DISTDIR}/${pkgfile}
done
