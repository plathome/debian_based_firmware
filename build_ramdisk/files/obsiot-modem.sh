#!/bin/bash
#
# Copyright (c) 2013-2017 Plat'Home CO., LTD.
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

GPIOPATH=/sys/class/gpio

[ -f /etc/default/openblocks ] && . /etc/default/openblocks
case $MODEL in
obsbx1)
	[ ! -e $GPIOPATH/gpio44 ] && echo 44 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio45 ] && echo 45 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio46 ] && echo 46 > $GPIOPATH/export

	echo in > $GPIOPATH/gpio44/direction
	echo in > $GPIOPATH/gpio45/direction
	echo in > $GPIOPATH/gpio46/direction

	id0=`cat $GPIOPATH/gpio44/value`
	id1=`cat $GPIOPATH/gpio45/value`
	id2=`cat $GPIOPATH/gpio46/value`

	echo 44 > $GPIOPATH/unexport
	echo 45 > $GPIOPATH/unexport
	echo 46 > $GPIOPATH/unexport
	;;
obsvx1)
	[ ! -e $GPIOPATH/gpio360 ] && echo 360 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio361 ] && echo 361 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio362 ] && echo 362 > $GPIOPATH/export

	echo in > $GPIOPATH/gpio360/direction
	echo in > $GPIOPATH/gpio361/direction
	echo in > $GPIOPATH/gpio362/direction
	echo 1 > $GPIOPATH/gpio360/active_low
	echo 1 > $GPIOPATH/gpio361/active_low
	echo 1 > $GPIOPATH/gpio362/active_low

	id0=`cat $GPIOPATH/gpio360/value`
	id1=`cat $GPIOPATH/gpio361/value`
	id2=`cat $GPIOPATH/gpio362/value`

	echo 360 > $GPIOPATH/unexport
	echo 361 > $GPIOPATH/unexport
	echo 362 > $GPIOPATH/unexport
	;;
*)
	id0="F"
	id1="F"
	id2="F"
	;;
esac

case $MODEL in
obsvx1)
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
	011)			# BX3 U200/U270
		echo "ERROR ($id2$id1$id0)"
		exit 8
		;;
	001)
		echo "S710E"
		exit 9
		;;
	*)
		echo "ERROR ($id2$id1$id0)"
		exit 8
		;;
	esac
	;;
obsbx1)
	ary=(`cat /proc/cmdline`)
	for i in `seq 1 ${#ary[@]}`
	do
		case ${ary[$i]} in
		modem*)
			IFS='='
			set -- ${ary[$i]}
			dipsw=$2
			;;
		esac
	done

	case $dipsw in
	70)
		echo "EHS6"
		exit 7
		;;
	50)
		echo "U200"
		exit 5
		;;
	10)
		echo "S710"
		exit 10
		;;
	60)
		echo "U200E"
		exit 6
		;;
	40)
		echo "KYM11"
		exit 4
		;;
	20)
		echo "UM04"
		exit 2
		;;
	0)
		echo "none"
		exit 0
		;;
	48)
		echo "S710E"
		exit 9
		;;
	*)
		echo "ERROR ($dipsw)"
		exit 8
		;;
	esac
	;;
esac
