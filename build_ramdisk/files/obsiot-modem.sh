#!/bin/bash

GPIOPATH=/sys/class/gpio

if [ ! -e $GPIOPATH/gpio44 ];then
	echo 44 > $GPIOPATH/export
fi
if [ ! -e $GPIOPATH/gpio45 ];then
	echo 45 > $GPIOPATH/export
fi
if [ ! -e $GPIOPATH/gpio46 ];then
	echo 46 > $GPIOPATH/export
fi

echo in > $GPIOPATH/gpio44/direction
echo in > $GPIOPATH/gpio45/direction
echo in > $GPIOPATH/gpio46/direction

id0=`cat $GPIOPATH/gpio44/value`
id1=`cat $GPIOPATH/gpio45/value`
id2=`cat $GPIOPATH/gpio46/value`

echo 44 > $GPIOPATH/unexport
echo 45 > $GPIOPATH/unexport
echo 46 > $GPIOPATH/unexport

case "$id2$id1$id0" in
000)			# EX1 none or BX0
	echo "none"
	exit 0
	;;
010)			# EX1 UM04-KO
	echo "UM04"
	exit 2
	;;
100)			# EX1 KYM11/12
	echo "KYM11"
	exit 4
	;;
110)			# EX1 U200/U270
	echo "U200E"
	exit 6
	;;
101)			# BX3 U200/U270
	echo "U200"
	exit 5
	;;
111)			# BX1 EHS6
	echo "EHS6"
	exit 7
	;;
*)
	echo "ERROR ($id2$id1$id0)"
	exit 8
	;;
esac
