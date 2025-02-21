#!/bin/sh
#
# Copyright (c) 2013-2025 Plat'Home CO., LTD.
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

#
# For OBS IoT EX1 Rev.2 and VX1
#
#    console :
#	usb	= BUS-Power IN
#	ac	= AC Adapter IN
#	dc	= Wide-DC IN
#	other	= ""
#
#    return value :
#	usb	= 1
#	ac	= 2
#	dc	= 3
#	other	= 0

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

GPIOPATH="/sys/class/gpio"
KERNEL_MAJOR_VERSION=`uname -r | cut -d '.' -f 1`
KERNEL_MINOR_VERSION=`uname -r | cut -d '.' -f 2`

case $MODEL in
obsvx*)
	if [ ${KERNEL_MAJOR_VERSION} = 6 -a ${KERNEL_MINOR_VERSION} = 12 ] ; then
		GPIOBASE=642
	elif [ ${KERNEL_MAJOR_VERSION} = 6 -a ${KERNEL_MINOR_VERSION} = 1 ] ; then
		GPIOBASE=850
	else
		GPIOBASE=338
	fi
	USBPOW=`expr ${GPIOBASE} + 28`
	ACPOW=`expr ${GPIOBASE} + 29`
	DCPOW=`expr ${GPIOBASE} + 27`
	;;
obsgem*)
	USBPOW=503
	ACPOW=500
	DCPOW=504
	;;
obsfx0*|obsfx1*)
	"$MODEL is not supported."
	exit 1
	;;
*)
	USBPOW=40
	ACPOW=41
	DCPOW=42
	;;
esac

if [ ! -e ${GPIOPATH}/gpio${USBPOW} ]; then
	echo ${USBPOW} > ${GPIOPATH}/export
fi
if [ ! -e ${GPIOPATH}/gpio${ACPOW} ]; then
	echo ${ACPOW} > ${GPIOPATH}/export
fi
if [ ! -e ${GPIOPATH}/gpio${DCPOW} ]; then
	echo ${DCPOW} > ${GPIOPATH}/export
fi
USBVAL=`cat ${GPIOPATH}/gpio${USBPOW}/value`
ACVAL=`cat ${GPIOPATH}/gpio${ACPOW}/value`
DCVAL=`cat ${GPIOPATH}/gpio${DCPOW}/value`
#echo $USBVAL $ACVAL $DCVAL

if [ "$MODEL" != "obsvx2" ]; then
	obs-hwclock --check
	[ $? = 255 ] && exit 0	# EX1 Rev1 or BX?(exclude BX0)
fi

val=`expr $USBVAL + $ACVAL + $DCVAL`
#echo "val=${val}"
if [ ${val} = 0 ];then	# BX0
	exit 0
fi

if [ ${ACVAL} = "1" ]; then
	echo "ac"
	exit 2
elif [ ${DCVAL} = "1" ]; then
	echo "dc"
	exit 3
elif [ ${USBVAL} = "1" ]; then
	echo "usb"
	exit 1
else
	exit 0
fi
