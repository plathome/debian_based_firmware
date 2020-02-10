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

case $TARGET in
bpv4*|obsmv4)
	BUILDDIR=/tmp/obstools.$$
	LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

	CFLAGS="-Wall -I/usr/${KERN_ARCH}-linux-gnu${ABI}/include -L/usr/${KERN_ARCH}-linux-gnu${ABI}/lib -DCONFIG_SBC8A361"

	mkdir -p ${BUILDDIR}
	echo "PSHD"
	$CC -o ${BUILDDIR}/pshd ${FILESDIR}/pshd_amd64.c $CFLAGS

	echo;echo;echo
	(cd ${BUILDDIR}; ls -l pshd)

	for cmd in pshd; do
		(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
		$STRIP ${DISTDIR}/usr/sbin/$cmd
	done
#	cp ${FILESDIR}/hwclock.sh ${DISTDIR}/usr/local/sbin/hwclock
#	chmod 555 ${DISTDIR}/usr/local/sbin/hwclock
#	cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
#	chmod 555 ${DISTDIR}/usr/sbin/flashcfg
;;
obsbx*)
	BUILDDIR=/tmp/obstools.$$
	LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

	CFLAGS="-Wall -I/usr/include/${ARCH}-linux-gnu${ABI}/ -L/usr/lib/${ARCH}-linux-gnu${ABI}/ -L/lib/${ARCH}-linux-gnu${ABI}/ -m32 -O2 -march=core2 -mtune=core2 -msse3 -mfpmath=sse -mstackrealign -fno-omit-frame-pointer -DCONFIG_OBSBX1"

	mkdir -p ${BUILDDIR}

	echo "MMC-UTILS"
	(cd ${FILESDIR}/mmc-utils;make clean;CFLAGS="$CFLAGS -m32" LDFLAGS="-m32" CC=gcc make;prefix=${DISTDIR}/usr make install)

	echo "HOSTAPD"
	if [ ${DIST} == "wheezy" ]; then
		if [ -d ${FILESDIR}/hostapd ]; then
			rm -f ${FILESDIR}/hostapd/hostapd/.config
			cp ${FILESDIR}/hostapd/hostapd/defconfig ${FILESDIR}/hostapd/hostapd/.config
			echo "CONFIG_IEEE80211N=y" >> ${FILESDIR}/hostapd/hostapd/.config
			echo "CONFIG_IEEE80211AC=y" >> ${FILESDIR}/hostapd/hostapd/.config
			echo "CONFIG_LIBNL32=y" >> ${FILESDIR}/hostapd/hostapd/.config
			(cd ${FILESDIR}/hostapd/hostapd;CFLAGS="$CFLAGS -MMD -I${DISTDIR}/usr/include" LDFLAGS="-m32 -L${DISTDIR}/usr/lib" CC=gcc make;DESTDIR=${DISTDIR} make install)
		fi
	fi

	if [ "$DIST" != "buster" ]; then
		apt-get -y install libi2c-dev:i386
		echo "WD-KEEPALIVE"
		$CC -o ${BUILDDIR}/wd-keepalive ${FILESDIR}/wd-keepalive.c $CFLAGS

		echo "OBS-UTIL"
		$CC -o ${BUILDDIR}/obs-util ${FILESDIR}/obs-util.c $CFLAGS

		echo "KOSANU"
		$CC -o ${BUILDDIR}/kosanu ${FILESDIR}/kosanu.c $CFLAGS
		$CC -o ${BUILDDIR}/runled ${FILESDIR}/runled_bx1.c $CFLAGS

		echo "PSHD"
		$CC -o ${BUILDDIR}/pshd ${FILESDIR}/pshd_bx1.c $CFLAGS

		echo "ATCMD"
		$CC -o ${BUILDDIR}/atcmd ${FILESDIR}/atcmd.c $CFLAGS

		echo "OBS-HWCLOCK"
		$CC -o ${BUILDDIR}/obs-hwclock ${FILESDIR}/obs-hwclock.c $CFLAGS

		echo "WAV-PLAY"
		_CFLAGS="$CFLAGS -lasound"
		$CC -o ${BUILDDIR}/wav-play ${FILESDIR}/wav-play.c $_CFLAGS

		echo "OBSIOT-POWER"
		[ "$DIST" == "buster" ] && CFLAGS="$CFLAGS -li2c"
		$CC -o ${BUILDDIR}/obsiot-power ${FILESDIR}/obsiot-power.c $CFLAGS

		cp ${FILESDIR}/obsiot-modem.sh ${DISTDIR}/usr/sbin/obsiot-modem.sh
		chmod 555 ${DISTDIR}/usr/sbin/obsiot-modem.sh
		cp ${FILESDIR}/obsiot-power.sh ${DISTDIR}/usr/sbin/obsiot-power.sh
		chmod 555 ${DISTDIR}/usr/sbin/obsiot-power.sh
		cp ${FILESDIR}/hwclock.sh ${DISTDIR}/usr/local/sbin/hwclock
		chmod 555 ${DISTDIR}/usr/local/sbin/hwclock
		cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
		chmod 555 ${DISTDIR}/usr/sbin/flashcfg
		cp ${FILESDIR}/retrieve_crashlog.sh ${DISTDIR}/usr/sbin/retrieve_crashlog.sh
		chmod 555 ${DISTDIR}/usr/sbin/retrieve_crashlog.sh
	fi
	echo "BRCM_PATCHRAM_PLUS"
	$CC -o ${BUILDDIR}/brcm_patchram_plus ${FILESDIR}/brcm_patchram_plus.c $CFLAGS

	echo "HUB-CTRL"
	apt-get -y install libusb-dev:i386
	_CFLAGS="$CFLAGS -lusb "
	$CC -o ${BUILDDIR}/hub-ctrl ${FILESDIR}/hub-ctrl.c $_CFLAGS

	echo "BLUETOOTH_RFKILL_EVENT"
	apt-get -y install libglib2.0-dev:i386
	_CFLAGS="$CFLAGS -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include -lglib-2.0"
	if [ "$DIST" == "stretch" ]; then
		_CFLAGS="$_CFLAGS -DCONFIG_LINUX4"
	fi
	$CC -o ${BUILDDIR}/bluetooth_rfkill_event ${FILESDIR}/bluetooth_rfkill_event.c $_CFLAGS

	echo;echo;echo
	(cd ${BUILDDIR}; ls -l hub-ctrl bluetooth_rfkill_event brcm_patchram_plus)

	for cmd in hub-ctrl bluetooth_rfkill_event brcm_patchram_plus; do
		(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
		$STRIP ${DISTDIR}/usr/sbin/$cmd
	done
	for cmd in fw_printenv; do
		(cd ${FILESDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
		$STRIP ${DISTDIR}/usr/sbin/$cmd
	done
	(cd ${DISTDIR}/usr/sbin; ln -sf fw_printenv fw_setenv)
	cp ${FILESDIR}/fw_env.config ${DISTDIR}/etc/
	cp ${FILESDIR}/chksignal.sh ${DISTDIR}/usr/sbin/
	chmod 555 ${DISTDIR}/usr/sbin/chksignal.sh
	(cd ${DISTDIR}/usr/local/bin; ln -sf /bin/busybox hwclock)
	if [ ${DIST} == "wheezy" ]; then
		(cd ${FILESDIR}; install -c -o root -g root -m 555 gatttool ${DISTDIR}/usr/local/sbin/gatttool)
		$STRIP ${DISTDIR}/usr/local/sbin/gatttool
	fi

	echo "CP2105-RS485"
	(cd ${FILESDIR}/cp210xmanufacturing;make clean;					\
		CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32 make;				\
		strip ${FILESDIR}/cp210xmanufacturing/Release/Linux/libcp210xmanufacturing.so.1.0
		cp ${FILESDIR}/cp210xmanufacturing/Release/Linux/libcp210xmanufacturing.so.1.0 ${DISTDIR}/usr/lib/i386-linux-gnu/libcp210xmanufacturing.so;	\
		cc -O2 -Wall -m32 -I./Release/Linux -I./Common -L./Release/Linux	\
		-lcp210xmanufacturing -o cp2105-rs485 cp2105-rs485.c;			\
		strip ${FILESDIR}/cp210xmanufacturing/cp2105-rs485
		cp ${FILESDIR}/cp210xmanufacturing/cp2105-rs485 ${DISTDIR}/usr/sbin
	)
	chroot ${DISTDIR} ldconfig
;;
obsvx*)
	BUILDDIR=/tmp/obstools.$$
	LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

	CFLAGS="-Wall -I/usr/include/${KERN_ARCH}-linux-gnu${ABI}/ -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -O2 -mstackrealign -fno-omit-frame-pointer -DCONFIG_OBSVX1"
	[ "$DIST" == "buster" ] && CFLAGS="$CFLAGS -li2c"

	mkdir -p ${BUILDDIR}

	echo "MMC-UTILS"
	(cd ${FILESDIR}/mmc-utils;make clean;CC=gcc make;prefix=${DISTDIR}/usr make install)

	apt-get -y install libi2c-dev
	if [ "$DIST" != "buster" ]; then
	echo "WD-KEEPALIVE"
	$CC -o ${BUILDDIR}/wd-keepalive ${FILESDIR}/wd-keepalive.c $CFLAGS

	echo "OBS-UTIL"
	$CC -o ${BUILDDIR}/obs-util ${FILESDIR}/obs-util.c $CFLAGS

	echo "KOSANU"
	$CC -o ${BUILDDIR}/kosanu ${FILESDIR}/kosanu.c $CFLAGS

	echo "RUNLED"
	$CC -o ${BUILDDIR}/runled ${FILESDIR}/runled_bx1.c $CFLAGS

	echo "PSHD"
	$CC -o ${BUILDDIR}/pshd ${FILESDIR}/pshd_bx1.c $CFLAGS

	echo "ATCMD"
	$CC -o ${BUILDDIR}/atcmd ${FILESDIR}/atcmd.c $CFLAGS

	echo "OBS-HWCLOCK"
	$CC -o ${BUILDDIR}/obs-hwclock ${FILESDIR}/obs-hwclock.c $CFLAGS

		echo "OBSIOT-POWER"
		$CC -o ${BUILDDIR}/obsiot-power ${FILESDIR}/obsiot-power.c $CFLAGS
		cp ${FILESDIR}/obsiot-power.sh ${DISTDIR}/usr/sbin/obsiot-power.sh
		chmod 555 ${DISTDIR}/usr/sbin/obsiot-power.sh

	echo "WAV-PLAY"
	_CFLAGS="$CFLAGS -lasound"
	$CC -o ${BUILDDIR}/wav-play ${FILESDIR}/wav-play.c $_CFLAGS

	echo "OBSVX1-MODEM"
	$CC -o ${BUILDDIR}/obsvx1-modem ${FILESDIR}/obsvx1-modem.c $CFLAGS

	echo "OBSVX1-GPIO"
	$CC -o ${BUILDDIR}/obsvx1-gpio ${FILESDIR}/obsvx1-gpio.c $CFLAGS

		cp ${FILESDIR}/obsiot-modem.sh ${DISTDIR}/usr/sbin/obsiot-modem.sh
		chmod 555 ${DISTDIR}/usr/sbin/obsiot-modem.sh
	fi

	echo "HUB-CTRL"
	apt-get -y install libusb-dev
	_CFLAGS="$CFLAGS -lusb "
	$CC -o ${BUILDDIR}/hub-ctrl ${FILESDIR}/hub-ctrl.c $_CFLAGS

	echo;echo;echo
	if [ "$DIST" == "buster" ]; then
		OBSTOOLLIST="hub-ctrl"
	else
		OBSTOOLLIST="wd-keepalive pshd runled kosanu atcmd hub-ctrl obs-util obs-hwclock wave-play obsvx1-modem obsvx1-gpio obsiot-power"
	fi
	(cd ${BUILDDIR}; ls -l ${OBSTOOLLIST})

	for cmd in ${OBSTOOLLIST}; do
		(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
		$STRIP ${DISTDIR}/usr/sbin/$cmd
	done

	cp ${FILESDIR}/chksignal.sh ${DISTDIR}/usr/sbin/
	chmod 555 ${DISTDIR}/usr/sbin/chksignal.sh
#	cp ${FILESDIR}/install-firmware.sh ${DISTDIR}/usr/sbin/install-firmware.sh
#	chmod 555 ${DISTDIR}/usr/sbin/install-firmware.sh
#	cp ${FILESDIR}/flashcfg-rootfs.sh ${DISTDIR}/usr/sbin/flashcfg
#	chmod 555 ${DISTDIR}/usr/sbin/flashcfg
	cp ${FILESDIR}/bin/resize ${DISTDIR}/usr/bin/
	chmod 555 ${DISTDIR}/usr/bin/resize


	echo "CP2104-RS485"
	(cd ${FILESDIR}/cp210xmanufacturing;make clean;make;				\
		strip ${FILESDIR}/cp210xmanufacturing/Release/Linux/libcp210xmanufacturing.so.1.0
		cp ${FILESDIR}/cp210xmanufacturing/Release/Linux/libcp210xmanufacturing.so.1.0 ${DISTDIR}/usr/lib/x86_64-linux-gnu/libcp210xmanufacturing.so;	\
		cc -O2 -Wall -I./Release/Linux -I./Common -L./Release/Linux		\
		-lcp210xmanufacturing -o cp2104-rs485 cp2104-rs485.c;			\
		strip ${FILESDIR}/cp210xmanufacturing/cp2104-rs485
		cp ${FILESDIR}/cp210xmanufacturing/cp2104-rs485 ${DISTDIR}/usr/sbin
	)
	chroot ${DISTDIR} ldconfig
;;
obsgem1)
	BUILDDIR=/tmp/obstools.$$
	LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

	CFLAGS="-Wall -I/usr/include/${KERN_ARCH}-linux-gnu${ABI}/ -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -O2 -fno-omit-frame-pointer -DCONFIG_OBSGEM1"

	mkdir -p ${BUILDDIR}

	echo "MMC-UTILS"
	(cd ${FILESDIR}/mmc-utils;make clean;CC=gcc make;prefix=${DISTDIR}/usr make install)

	apt-get -y install libi2c-dev:arm64
	echo "WD-KEEPALIVE"
	$CC -o ${BUILDDIR}/wd-keepalive ${FILESDIR}/wd-keepalive.c $CFLAGS

	echo "OBS-UTIL"
	$CC -o ${BUILDDIR}/obs-util ${FILESDIR}/obs-util.c $CFLAGS

	echo "KOSANU"
	$CC -o ${BUILDDIR}/kosanu ${FILESDIR}/kosanu.c $CFLAGS

	echo "RUNLED"
	$CC -o ${BUILDDIR}/runled ${FILESDIR}/runled_bx1.c $CFLAGS

	echo "PSHD"
	$CC -o ${BUILDDIR}/pshd ${FILESDIR}/pshd_bx1.c $CFLAGS

	echo "ATCMD"
	$CC -o ${BUILDDIR}/atcmd ${FILESDIR}/atcmd.c $CFLAGS

	echo "OBS-HWCLOCK"
	$CC -o ${BUILDDIR}/obs-hwclock ${FILESDIR}/obs-hwclock.c $CFLAGS

#	echo "HUB-CTRL"
#	apt-get -y install libusb-dev
#	_CFLAGS="$CFLAGS -lusb "
#	$CC -o ${BUILDDIR}/hub-ctrl ${FILESDIR}/hub-ctrl.c $_CFLAGS

	echo "WAV-PLAY"
	_CFLAGS="$CFLAGS -lasound"
	$CC -o ${BUILDDIR}/wav-play ${FILESDIR}/wav-play.c $_CFLAGS

	echo "OBSIOT-POWER"
	$CC -o ${BUILDDIR}/obsiot-power ${FILESDIR}/obsiot-power.c $CFLAGS

	echo;echo;echo
#	(cd ${BUILDDIR}; ls -l wd-keepalive pshd runled kosanu atcmd hub-ctrl obs-util obs-hwclock wav-play obsiot-power)
	(cd ${BUILDDIR}; ls -l wd-keepalive pshd runled kosanu atcmd obs-util obs-hwclock wav-play obsiot-power)

#	for cmd in wd-keepalive pshd runled kosanu atcmd hub-ctrl obs-util obs-hwclock wav-play obsiot-power; do
	for cmd in wd-keepalive pshd runled kosanu atcmd obs-util obs-hwclock wav-play obsiot-power; do
		(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
		$STRIP ${DISTDIR}/usr/sbin/$cmd
	done

	cp ${FILESDIR}/obsiot-modem.sh ${DISTDIR}/usr/sbin/obsiot-modem.sh
	chmod 555 ${DISTDIR}/usr/sbin/obsiot-modem.sh
	cp ${FILESDIR}/obsiot-power.sh ${DISTDIR}/usr/sbin/obsiot-power.sh
	chmod 555 ${DISTDIR}/usr/sbin/obsiot-power.sh
	cp ${FILESDIR}/hwclock.sh ${DISTDIR}/usr/local/sbin/hwclock
	chmod 555 ${DISTDIR}/usr/local/sbin/hwclock
	cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
	chmod 555 ${DISTDIR}/usr/sbin/flashcfg

	;;
obsix*)
	BUILDDIR=/tmp/obstools.$$
	LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

	CFLAGS="-Wall -I/usr/include/${KERN_ARCH}-linux-gnu${ABI}/ -L/usr/lib/${KERN_ARCH}-linux-gnu${ABI}/ -O2 -mstackrealign -fno-omit-frame-pointer -li2c -DCONFIG_OBSIX9"
	mkdir -p ${BUILDDIR}

	echo "MMC-UTILS"
	(cd ${FILESDIR}/mmc-utils;make clean;CC=gcc make;prefix=${DISTDIR}/usr make install)

	echo "HUB-CTRL"
	apt-get -y install libusb-dev
	_CFLAGS="$CFLAGS -lusb "
	$CC -o ${BUILDDIR}/hub-ctrl ${FILESDIR}/hub-ctrl.c $_CFLAGS

	echo;echo;echo
	OBSTOOLLIST="hub-ctrl"
	(cd ${BUILDDIR}; ls -l ${OBSTOOLLIST})

	for cmd in ${OBSTOOLLIST}; do
		(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
		$STRIP ${DISTDIR}/usr/sbin/$cmd
	done

#	cp ${FILESDIR}/install-firmware.sh ${DISTDIR}/usr/sbin/install-firmware.sh
#	chmod 555 ${DISTDIR}/usr/sbin/install-firmware.sh
#	cp ${FILESDIR}/flashcfg-rootfs.sh ${DISTDIR}/usr/sbin/flashcfg
#	chmod 555 ${DISTDIR}/usr/sbin/flashcfg
	cp ${FILESDIR}/bin/resize ${DISTDIR}/usr/bin/
	chmod 555 ${DISTDIR}/usr/bin/resize
;;
*)
;;
esac

rm -rf ${BUILDDIR}

