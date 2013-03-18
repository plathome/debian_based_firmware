#!/bin/bash

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

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
