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

case $DIST in
buster);;
*) exit;;
esac

[ ! -d $RELEASEDIR ] && mkdir -p $RELEASEDIR

case $TARGET in
obsvx*)
	pkglist="atcmd disable_modem nitz obs_util obs_hwclock obsiot_power obsvx1_modem obsvx1_gpio pshd runled wav_play wd_keepalive"
	;;
obsbx*)
	pkglist="atcmd disable_modem nitz obs_util obs_hwclock obsiot_power pshd runled wav_play wd_keepalive"
	;;
obsix*)
	pkglist="obs_util obs_hwclock pshd runled wav_play wd_keepalive"
	;;
obsgem*)
	pkglist="atcmd disable_modem nitz obs_util obs_hwclock obsiot_power pshd runled wav_play"
	;;
*) exit 1 ;;
esac

LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include
apt-get -y install libi2c-dev

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
obsix*)
	CFLAGS="-Wall -I/usr/${KERN_ARCH}-linux-gnu${ABI}/include -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -li2c -O2 -mstackrealign -fno-omit-frame-pointer -DCONFIG_OBSIX9"
	;;
*) exit 1 ;;
esac

#
# per arch
#

case $TARGET in
obsbx*|obsvx*|obsgem*)
	echo "ATCMD"
	$CC -o ${OBSTOOLDIR}/template-atcmd/usr/sbin/atcmd ${FILESDIR}/atcmd.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-atcmd/usr/sbin/atcmd
	cp -f ${FILESDIR}/obsiot-modem.sh ${OBSTOOLDIR}/template-atcmd/usr/sbin/obsiot-modem.sh
	chmod 555 ${OBSTOOLDIR}/template-atcmd/usr/sbin/obsiot-modem.sh

	echo "NITZ"
	cp -f ${FILESDIR}/nitz.sh ${OBSTOOLDIR}/template-nitz/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-nitz/usr/sbin/nitz.sh

	echo "DISABLE-MODEM"
	cp -f ${FILESDIR}/disable-modem.sh ${OBSTOOLDIR}/template-disable-modem/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-disable-modem/usr/sbin/disable-modem.sh
	;;
esac

case $TARGET in
obsbx*|obsvx*|obsgem*)
	echo "OBS-HWCLOCK"
	$CC -o ${OBSTOOLDIR}/template-obs-hwclock/usr/sbin/obs-hwclock ${FILESDIR}/obs-hwclock.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obs-hwclock/usr/sbin/obs-hwclock
	cp -f ${FILESDIR}/hwclock.sh ${OBSTOOLDIR}/template-obs-hwclock/usr/local/sbin/hwclock
	chmod 555 ${OBSTOOLDIR}/template-obs-hwclock/usr/local/sbin/hwclock
	;;
esac

echo "OBS-UTIL"
$CC -o ${OBSTOOLDIR}/template-obs-util/usr/sbin/obs-util ${FILESDIR}/obs-util.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-obs-util/usr/sbin/obs-util
$CC -o ${OBSTOOLDIR}/template-obs-util/usr/sbin/kosanu ${FILESDIR}/kosanu.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-obs-util/usr/sbin/kosanu

case $TARGET in
obsbx*|obsvx*|obsgem*)
	echo "OBSIOT-POWER"
	$CC -o ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/obsiot-power ${FILESDIR}/obsiot-power.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/obsiot-power
	cp -f ${FILESDIR}/obsiot-power.sh ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/obsiot-power.sh

	;;
esac

case $TARGET in
obsvx*)
	echo "OBSVX1-MODEM"
	$CC -o ${OBSTOOLDIR}/template-obsvx1-modem/usr/sbin/obsvx1-modem ${FILESDIR}/obsvx1-modem.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsvx1-modem/usr/sbin/obsvx1-modem

	echo "OBSVX1-GPIO"
	$CC -o ${OBSTOOLDIR}/template-obsvx1-gpio/usr/sbin/obsvx1-gpio ${FILESDIR}/obsvx1-gpio.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsvx1-gpio/usr/sbin/obsvx1-gpio
	;;
esac

echo "RUNLED"
$CC -o ${OBSTOOLDIR}/template-runled/usr/sbin/runled ${FILESDIR}/runled_bx1.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-runled/usr/sbin/runled
cp -f ${FILESDIR}/setup-runled.sh ${OBSTOOLDIR}/template-runled/usr/sbin/
chmod 555 ${OBSTOOLDIR}/template-runled/usr/sbin/setup-runled.sh

echo "PSHD"
$CC -o ${OBSTOOLDIR}/template-pshd/usr/sbin/pshd ${FILESDIR}/pshd_bx1.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-pshd/usr/sbin/pshd

echo "WAV-PLAY"
_CFLAGS="$CFLAGS -lasound"
$CC -o ${OBSTOOLDIR}/template-wav-play/usr/sbin/wav-play ${FILESDIR}/wav-play.c $_CFLAGS
$STRIP ${OBSTOOLDIR}/template-wav-play/usr/sbin/wav-play

case $TARGET in
obsix*|obsvx*|obsbx*)
	echo "WD-KEEPALIVE"
	$CC -o ${OBSTOOLDIR}/template-wd-keepalive/usr/sbin/wd-keepalive ${FILESDIR}/wd-keepalive.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-wd-keepalive/usr/sbin/wd-keepalive
	;;
esac

for pkg in $pkglist; do
	eval version='$'${pkg}_ver
	pkg=${pkg//_/-}
	pkgfile=${pkg}-${version}-${ARCH}.deb
	rm -f ${RELEASEDIR}/${pkgfile}
	(cd ${OBSTOOLDIR}/; ./mkdeb.sh ${version} ${ARCH} ${pkg} ${RELEASEDIR} "")
	cp -f ${RELEASEDIR}/${pkgfile} ${DISTDIR}/
	chroot ${DISTDIR} dpkg -r ${pkg}
	chroot ${DISTDIR} dpkg -i ${pkgfile}
	rm -f ${DISTDIR}/${pkgfile}
done

#
# all
#
pkglist="instfirm setup_gpio"

echo "SETUP-GPIO"
cp -f ${FILESDIR}/setup-gpio.sh ${OBSTOOLDIR}/template-setup-gpio/usr/sbin/
chmod 555 ${OBSTOOLDIR}/template-setup-gpio/usr/sbin/setup-gpio.sh

case $TARGET in
obsvx*|obsix*)
	echo "FLASHCFG"
	cp -f ${FILESDIR}/flashcfg-rootfs.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	cp -f ${FILESDIR}/instfirm.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/instfirm.sh
	;;
obsbx*)
	echo "FLASHCFG"
	cp -f ${FILESDIR}/flashcfg.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	;;
esac

for pkg in $pkglist; do
	eval version='$'${pkg}_ver
	pkg=${pkg//_/-}
	pkgfile=${pkg}-${version}-all.deb
	rm -f ${RELEASEDIR}/${pkgfile}
	(cd ${OBSTOOLDIR}/; ./mkdeb.sh ${version} "all" ${pkg} ${RELEASEDIR} "")
	cp -f ${RELEASEDIR}/${pkgfile} ${DISTDIR}/
	chroot ${DISTDIR} dpkg -r ${pkg}
	chroot ${DISTDIR} dpkg -i ${pkgfile}
	chroot ${DISTDIR} ln -sf /lib/systemd/system/${pkg}.service /etc/systemd/system/multi-user.target.wants/${pkg}.service
	rm -f ${DISTDIR}/${pkgfile}
done
