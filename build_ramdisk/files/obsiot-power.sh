#!/bin/sh
#
# Only OBS IoT EX1 Rev.2
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

GPIOPATH="/sys/class/gpio"
USBPOW=40
ACPOW=41
DCPOW=42

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

obs-hwclock --check
if [ $? = 255 ]; then	# EX1 Rev1 or BX?(exclude BX0)
	exit 0
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
