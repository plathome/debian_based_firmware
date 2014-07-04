#!/bin/bash

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

if [ $# == 0 ]; then
	if [ -f /sys/class/gpio/export ] ; then
		for port in $USBREST_GPIO_PORTS;do
			echo $port > /sys/class/gpio/export
			echo out > /sys/class/gpio/gpio${port}/direction
			echo 0 > /sys/class/gpio/gpio${port}/value
		done
		sleep 1
		for port in $USBREST_GPIO_PORTS;do
			echo 1 > /sys/class/gpio/gpio${port}/value
			echo $port > /sys/class/gpio/unexport
		done
	fi
	exit 0
fi

if [ $1 == 0 -o $1 == 1 ]; then
	num=`expr $1 + 1`
	if [ -f /sys/class/gpio/export ] ; then
		ary=(`echo $USBREST_GPIO_PORTS`)
		for i in `seq 1 ${#ary[@]}`;do
			if [ $num == $i ]; then
				echo ${ary[$i-1]} > /sys/class/gpio/export
				echo out > /sys/class/gpio/gpio${ary[$i-1]}/direction
				echo 0 > /sys/class/gpio/gpio${ary[$i-1]}/value
				sleep 1
				echo 1 > /sys/class/gpio/gpio${ary[$i-1]}/value
				echo ${ary[$i-1]} > /sys/class/gpio/unexport
				break
			fi
		done
	fi
else
	echo "usage: $0 portNo."
	echo "       portNo. = 0 or 1"
	exit 1
fi
