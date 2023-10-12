#! /bin/bash
#
# Copyright (c) 2013-2023 Plat'Home CO., LTD.
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

[ -r /etc/default/openblocks ] && . /etc/default/openblocks

GPIOPATH="/sys/class/gpio"
KERNEL_MAJOR_VERSION=`uname -r | cut -d '.' -f 1`
case $MODEL in
obsvx*|obsix9)
	if [ ${KERNEL_MAJOR_VERSION} -ge 6 ] ; then
		GPIOBASE=850
	else
		GPIOBASE=338
	fi
	OFFSET4=`expr ${GPIOBASE} + 4`
	OFFSET5=`expr ${GPIOBASE} + 5`
	OFFSET6=`expr ${GPIOBASE} + 6`
	OFFSET7=`expr ${GPIOBASE} + 7`
	OFFSET27=`expr ${GPIOBASE} + 27`
	OFFSET28=`expr ${GPIOBASE} + 28`
	OFFSET29=`expr ${GPIOBASE} + 29`

	# runled
	[ ! -d $GPIOPATH/gpio${OFFSET4} ] && echo ${OFFSET4} > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio${OFFSET4}/direction
	[ ! -d $GPIOPATH/gpio${OFFSET5} ] && echo ${OFFSET5} > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio${OFFSET4}/direction
	[ ! -d $GPIOPATH/gpio${OFFSET6} ] && echo ${OFFSET6} > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio${OFFSET4}/direction

	# pshd
	[ ! -d $GPIOPATH/gpio${OFFSET7} ] && echo ${OFFSET7} > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio${OFFSET7}/direction &&
		echo both > $GPIOPATH/gpio${OFFSET7}/edge

	# obsiot-power
	[ ! -d $GPIOPATH/gpio${OFFSET27} ] && echo ${OFFSET27} > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio${OFFSET28} ] && echo ${OFFSET28} > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio${OFFSET29} ] && echo ${OFFSET29} > $GPIOPATH/export
	;;
obsbx*)
	# runled
	[ ! -d $GPIOPATH/gpio47 ] && echo 47 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio47/direction
	[ ! -d $GPIOPATH/gpio48 ] && echo 48 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio48/direction
	[ ! -d $GPIOPATH/gpio49 ] && echo 49 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio49/direction

	# pshd
	[ ! -d $GPIOPATH/gpio14 ] && echo 14 > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio14/direction &&
		echo both > $GPIOPATH/gpio14/edge

	# obsiot-power
	[ ! -d $GPIOPATH/gpio40 ] && echo 40 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio41 ] && echo 41 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio42 ] && echo 42 > $GPIOPATH/export
	;;
obsa16*)
	# pshd
	[ ! -d $GPIOPATH/gpio86 ] && echo 86 > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio86/direction &&
		echo both > $GPIOPATH/gpio86/edge
	;;
obsfx0*)
	# USB1_PWR_EN
	[ ! -d $GPIOPATH/gpio0 ] && echo 0 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio0/direction 
	echo 1 > $GPIOPATH/gpio0/value 
	# WL_DIS_N
	[ ! -d $GPIOPATH/gpio10 ] && echo 10 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio10/direction
	echo 1 > $GPIOPATH/gpio10/value 
	# BT_DIS_N
	[ ! -d $GPIOPATH/gpio11 ] && echo 11 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio11/direction
	echo 1 > $GPIOPATH/gpio11/value 
	# POWERLOST
	[ ! -d $GPIOPATH/gpio13 ] && echo 13 > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio13/direction
	# USB1_PWR_EN
	[ ! -d $GPIOPATH/gpio14 ] && echo 14 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio14/direction
	echo 1 > $GPIOPATH/gpio14/value 
	# pshd
	[ ! -d $GPIOPATH/gpio86 ] && echo 86 > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio86/direction &&
		echo both > $GPIOPATH/gpio86/edge
	;;
obsfx1*)
	# pshd
	[ ! -d $GPIOPATH/gpio86 ] && echo 86 > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio86/direction &&
		echo both > $GPIOPATH/gpio86/edge
	# i2c-gpio
		# 496	WL_PWR_EN
		# 497	WL_DIS_N
		# 498	BT_DIS_N
		# 499	S1_PWR_EN
		# 500	S1_PON
	[ ! -d $GPIOPATH/gpio501 ] && echo 501 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio501/direction			# S1_RST_N
	echo 1 > $GPIOPATH/gpio501/value
	[ ! -d $GPIOPATH/gpio502 ] && echo 502 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio502/direction			# W_DISABLE
	echo 0 > $GPIOPATH/gpio502/value
	[ ! -d $GPIOPATH/gpio503 ] && echo 503 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio503/direction			# FULL_CD_POFF
	echo 0 > $GPIOPATH/gpio503/value
	[ ! -d $GPIOPATH/gpio504 ] && echo 504 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio504/direction			# S2_PWR_EN
	echo 1 > $GPIOPATH/gpio504/value
	[ ! -d $GPIOPATH/gpio505 ] && echo 505 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio505/direction			# S2_RST_N
	echo 1 > $GPIOPATH/gpio505/value
	[ ! -d $GPIOPATH/gpio506 ] && echo 506 > $GPIOPATH/export &&
		echo out > $GPIOPATH/gpio506/direction			# USBHUB_RST_N
	echo 1 > $GPIOPATH/gpio506/value
	[ ! -d $GPIOPATH/gpio507 ] && echo 507 > $GPIOPATH/export &&
		echo in > $GPIOPATH/gpio507/direction			# PWRFAIL_N
	# 508 D.C.
	# 509 D.C.
	# 510 D.C.
	# 511 N.C.
	;;
*)
	exit 1
	;;
esac

DEBIAN_MAJOR_VERSION=`cat /etc/debian_version | cut -d '.' -f 1`
case $DEBIAN_MAJOR_VERSION in
11|12) ;;
*)
	export DEBIAN_FRONTEND=noninteractive
	dpkg-reconfigure openssh-server
	;;
esac

exit 0
