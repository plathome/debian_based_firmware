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

. `dirname $0`/config.sh
. `dirname $0`/_obstool_version.sh

case $DIST in
buster|bullseye);;
*) exit;;
esac

[ ! -d $RELEASEDIR ] && mkdir -p $RELEASEDIR

case $TARGET in
obsvx1)
	pkglist="atcmd disable_modem nitz obs_util obs_hwclock obsvx1_modem obsvx1_gpio pshd runled wav_play wd_keepalive"
	;;
obsvx2)
	pkglist="atcmd disable_modem nitz obs_util obs_hwclock obsiot_power obsvx1_modem obsvx1_gpio pshd runled wav_play wd_keepalive"
	;;
obsbx*)
	pkglist="atcmd disable_modem nitz obs_util obs_hwclock obsiot_power pshd runled wav_play wd_keepalive"
	;;
obsix*)
	case $DIST in
	bullseye)
		pkglist="obs_util obs_hwclock obs_nicrename pshd runled wav_play wd_keepalive"
		;;
	*)
		pkglist="obs_util obs_hwclock pshd runled wav_play wd_keepalive"
		;;
	esac
	;;
obsa16*)
	pkglist="obs_util obs_hwclock pshd runled wav_play wd_keepalive"
	;;
obsfx1*)
	pkglist="atcmd cp210x_rs485 obs_util obs_hwclock obsiot_power obsfx1_modem pshd runled wav_play wd_keepalive"
	;;
obshx2*) exit 0 ;;
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
obsa16*)
	CFLAGS="-Wall -I/usr/include/${KERN_ARCH}-linux-gnu${ABI}/ -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -O2 -fno-omit-frame-pointer -DCONFIG_OBSA16"
	;;
obsfx1*)
	CFLAGS="-Wall -I/usr/include/${KERN_ARCH}-linux-gnu${ABI}/ -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -O2 -fno-omit-frame-pointer -DCONFIG_OBSA16 -DCONFIG_OBSFX1 "
	;;
obsix*)
	CFLAGS="-Wall -I/usr/${KERN_ARCH}-linux-gnu${ABI}/include -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -li2c -O2 -mstackrealign -fno-omit-frame-pointer -DCONFIG_OBSIX9"
	;;
obshx2*) exit 0 ;;
*) exit 1 ;;
esac

#
# per arch
#

case $TARGET in
obsbx*|obsvx*|obsfx1*)
	echo "ATCMD"
	mkdir -p ${OBSTOOLDIR}/template-atcmd/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-atcmd/usr/sbin/atcmd ${FILESDIR}/atcmd.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-atcmd/usr/sbin/atcmd
	cp -f ${FILESDIR}/obsiot-modem.sh ${OBSTOOLDIR}/template-atcmd/usr/sbin/obsiot-modem.sh
	mkdir -p ${OBSTOOLDIR}/template-atcmd/etc/udev/rules.d/
	cp -a ${ETCDIR_ADD}/udev/rules.d/5[0-9]-* ${OBSTOOLDIR}/template-atcmd/etc/udev/rules.d/
	chmod 555 ${OBSTOOLDIR}/template-atcmd/usr/sbin/obsiot-modem.sh

	echo "NITZ"
	mkdir -p ${OBSTOOLDIR}/template-nitz/usr/sbin/
	cp -f ${FILESDIR}/nitz.sh ${OBSTOOLDIR}/template-nitz/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-nitz/usr/sbin/nitz.sh

	echo "DISABLE-MODEM"
	mkdir -p ${OBSTOOLDIR}/template-disable-modem/usr/sbin/
	cp -f ${FILESDIR}/disable-modem.sh ${OBSTOOLDIR}/template-disable-modem/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-disable-modem/usr/sbin/disable-modem.sh
	;;
esac

case $TARGET in
obsbx*|obsvx*|obsa16*|obsfx1*)
	echo "OBS-HWCLOCK"
	mkdir -p ${OBSTOOLDIR}/template-obs-hwclock/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-obs-hwclock/usr/sbin/obs-hwclock ${FILESDIR}/obs-hwclock.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obs-hwclock/usr/sbin/obs-hwclock
	mkdir -p ${OBSTOOLDIR}/template-obs-hwclock/usr/local/sbin/
	cp -f ${FILESDIR}/hwclock.sh ${OBSTOOLDIR}/template-obs-hwclock/usr/local/sbin/hwclock
	chmod 555 ${OBSTOOLDIR}/template-obs-hwclock/usr/local/sbin/hwclock
	;;
esac

echo "OBS-UTIL"
mkdir -p ${OBSTOOLDIR}/template-obs-util/usr/sbin/
$CC -o ${OBSTOOLDIR}/template-obs-util/usr/sbin/obs-util ${FILESDIR}/obs-util.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-obs-util/usr/sbin/obs-util
$CC -o ${OBSTOOLDIR}/template-obs-util/usr/sbin/kosanu ${FILESDIR}/kosanu.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-obs-util/usr/sbin/kosanu

case $TARGET in
obsbx*|obsvx*|obsfx1*)
	echo "OBSIOT-POWER"
	mkdir -p ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/obsiot-power ${FILESDIR}/obsiot-power.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/obsiot-power
	cp -f ${FILESDIR}/obsiot-power.sh ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-obsiot-power/usr/sbin/obsiot-power.sh

	;;
esac

case $TARGET in
obsix*)
	echo "OBS-NICRENAME"
	mkdir -p ${OBSTOOLDIR}/template-obs-nicrename/usr/local/sbin/
	cp -f ${FILESDIR}/obs-nicrename.sh ${OBSTOOLDIR}/template-obs-nicrename/usr/local/sbin/
	chmod 555 ${OBSTOOLDIR}/template-obs-nicrename/usr/local/sbin/obs-nicrename.sh
	;;
esac

case $TARGET in
obsvx*)
	echo "OBSVX1-MODEM"
	mkdir -p ${OBSTOOLDIR}/template-obsvx1-modem/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-obsvx1-modem/usr/sbin/obsvx1-modem ${FILESDIR}/obsvx1-modem.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsvx1-modem/usr/sbin/obsvx1-modem

	echo "OBSVX1-GPIO"
	mkdir -p ${OBSTOOLDIR}/template-obsvx1-gpio/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-obsvx1-gpio/usr/sbin/obsvx1-gpio ${FILESDIR}/obsvx1-gpio.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsvx1-gpio/usr/sbin/obsvx1-gpio
	;;
obsfx1*)
	echo "OBSFX1-MODEM"
	mkdir -p ${OBSTOOLDIR}/template-obsfx1-modem/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-obsfx1-modem/usr/sbin/obsfx1-modem ${FILESDIR}/obsfx1-modem.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-obsfx1-modem/usr/sbin/obsfx1-modem
	;;
esac

echo "RUNLED"
mkdir -p ${OBSTOOLDIR}/template-runled/usr/sbin/
$CC -o ${OBSTOOLDIR}/template-runled/usr/sbin/runled ${FILESDIR}/runled_bx1.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-runled/usr/sbin/runled
cp -f ${FILESDIR}/setup-runled.sh ${OBSTOOLDIR}/template-runled/usr/sbin/
chmod 555 ${OBSTOOLDIR}/template-runled/usr/sbin/setup-runled.sh

echo "PSHD"
mkdir -p ${OBSTOOLDIR}/template-pshd/usr/sbin/
$CC -o ${OBSTOOLDIR}/template-pshd/usr/sbin/pshd ${FILESDIR}/pshd_bx1.c $CFLAGS
$STRIP ${OBSTOOLDIR}/template-pshd/usr/sbin/pshd

echo "WAV-PLAY"
_CFLAGS="$CFLAGS -lasound"
mkdir -p ${OBSTOOLDIR}/template-wav-play/usr/sbin/
$CC -o ${OBSTOOLDIR}/template-wav-play/usr/sbin/wav-play ${FILESDIR}/wav-play.c $_CFLAGS
$STRIP ${OBSTOOLDIR}/template-wav-play/usr/sbin/wav-play

case $TARGET in
obsix*|obsvx*|obsbx*|obsa16*|obsfx1*)
	echo "WD-KEEPALIVE"
	mkdir -p ${OBSTOOLDIR}/template-wd-keepalive/usr/sbin/
	$CC -o ${OBSTOOLDIR}/template-wd-keepalive/usr/sbin/wd-keepalive ${FILESDIR}/wd-keepalive.c $CFLAGS
	$STRIP ${OBSTOOLDIR}/template-wd-keepalive/usr/sbin/wd-keepalive
	;;
esac

for pkg in $pkglist; do
	eval version='$'${pkg}_ver
	pkg=${pkg//_/-}
	pkgfile=${pkg}-${version}-${ARCH}.deb
#	rm -f ${RELEASEDIR}/${pkgfile}
	rm -f ${RELEASEDIR}/${pkg}-*.deb
	(cd ${OBSTOOLDIR}/; ./mkdeb.sh ${version} ${ARCH} ${pkg} ${RELEASEDIR} "")
	cp -f ${RELEASEDIR}/${pkgfile} ${DISTDIR}/
	chroot ${DISTDIR} dpkg -r ${pkg}
	chroot ${DISTDIR} dpkg -i ${pkgfile}
	rm -f ${DISTDIR}/${pkgfile}
done

if [ "$TARGET" == "obsbx1" ] && [ "$DIST" == "bullseye" ]; then
	chroot ${DISTDIR} systemctl disable wd-keepalive 
fi

#
# all
#
case $TARGET in
obsvx*|obsix*)
	pkglist="instfirm obs_createkeys setup_gpio"
	;;
obsa16*|obsfx1*)
	pkglist="obs_createkeys setup_macether setup_gpio"
	;;
*)
	pkglist="obs_createkeys setup_gpio"
	;;
esac

echo "SETUP-GPIO"
mkdir -p ${OBSTOOLDIR}/template-setup-gpio/usr/sbin/
cp -f ${FILESDIR}/setup-gpio.sh ${OBSTOOLDIR}/template-setup-gpio/usr/sbin/
chmod 555 ${OBSTOOLDIR}/template-setup-gpio/usr/sbin/setup-gpio.sh

case $TARGET in
obsvx*|obsix9|obsa16*|obsfx1*)
	echo "FLASHCFG"
	FLASHCFG=flashcfg-rootfs.sh
	[ "$TARGET" == "obsvx1" -o "$TARGET" == "obsix9r" ] && FLASHCFG=flashcfg.sh
	mkdir -p ${OBSTOOLDIR}/template-instfirm/usr/sbin/
	cp -f ${FILESDIR}/${FLASHCFG} ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	cp -f ${FILESDIR}/instfirm.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/instfirm.sh
	;;
obsbx*|obsa16r|obsfx1r)
	echo "FLASHCFG"
	mkdir -p ${OBSTOOLDIR}/template-instfirm/usr/sbin/
	cp -f ${FILESDIR}/flashcfg.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	;;
obsix9r)
	echo "FLASHCFG"
	mkdir -p ${OBSTOOLDIR}/template-instfirm/usr/sbin/
	cp -f ${FILESDIR}/flashcfg.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/flashcfg
	cp -f ${FILESDIR}/instfirm.sh ${OBSTOOLDIR}/template-instfirm/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-instfirm/usr/sbin/instfirm.sh
	;;
esac

case $DIST in
bullseye|buster)
	echo "OBS-CREATEKEYS"
	mkdir -p ${OBSTOOLDIR}/template-obs-createkeys/usr/sbin/
	cp -f ${FILESDIR}/obs-createkeys.sh ${OBSTOOLDIR}/template-obs-createkeys/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-obs-createkeys/usr/sbin/obs-createkeys.sh
	;;
esac

case $TARGET in
obsa16*|obsfx1*)
	echo "OBS-MACADDR"
	mkdir -p ${OBSTOOLDIR}/template-obs-macaddr/usr/sbin/
	cp -f ${FILESDIR}/obs-macaddr.sh ${OBSTOOLDIR}/template-obs-macaddr/usr/sbin/
	chmod 555 ${OBSTOOLDIR}/template-obs-macaddr/usr/sbin/obs-macaddr.sh

	echo "SETUP-MACETHER"
	mkdir -p ${OBSTOOLDIR}/template-setup-macether/usr/local/sbin/
	cp -f ${FILESDIR}/setup-macether.sh ${OBSTOOLDIR}/template-setup-macether/usr/local/sbin/
	chmod 555 ${OBSTOOLDIR}/template-setup-macether/usr/local/sbin/setup-macether.sh
	;;
esac

for pkg in $pkglist; do
	eval version='$'${pkg}_ver
	pkg=${pkg//_/-}
	pkgfile=${pkg}-${version}-all.deb
#	rm -f ${RELEASEDIR}/${pkgfile}
	rm -f ${RELEASEDIR}/${pkg}-*.deb
	(cd ${OBSTOOLDIR}/; ./mkdeb.sh ${version} "all" ${pkg} ${RELEASEDIR} "")
	cp -f ${RELEASEDIR}/${pkgfile} ${DISTDIR}/
	chroot ${DISTDIR} dpkg -r ${pkg}
	chroot ${DISTDIR} dpkg -i ${pkgfile}
	[ -f /lib/systemd/system/${pkg}.service ] && \
		chroot ${DISTDIR} ln -sf /lib/systemd/system/${pkg}.service /etc/systemd/system/multi-user.target.wants/${pkg}.service
	rm -f ${DISTDIR}/${pkgfile}
done

case $TARGET in
obsa16*)
	chroot ${DISTDIR} systemctl disable wpa_supplicant
	;;
esac

case $TARGET in
obsbx1*)
	if [ "$DIST" = "bullseye"  ] ; then
		echo "BLUETOOTHD for bullseye(later?)"
		mkdir -p ${DISTDIR}/usr/local/sbin/
		cp -f ${FILESDIR}/bluez_5.50-1.2~deb10u2_i386/bluetoothd ${DISTDIR}/usr/local/sbin/
		mkdir -p ${DISTDIR}/lib/systemd/system/bluetooth.service.d
		{
			cat <<!
[Service]
ExecStart=
ExecStart=/usr/local/sbin/bluetoothd --noplugin=sap
!
		} > ${DISTDIR}/lib/systemd/system/bluetooth.service.d/override.conf
	fi
	;;
esac
