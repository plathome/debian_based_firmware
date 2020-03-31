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

function _call_obsutil() {
	TMP=/tmp/tmp.$$
	/usr/sbin/obs-util -m $TMP
	if [ ! -f $TMP ]; then
		exit
	fi
	IFS=$'\n'
	LIST=(`cat $TMP | fold -w2`)
	case ${LIST[0]} in
	01|02)
		if [ "$1" != "obsbx1" ]; then
			echo "ERROR${LINENO}"
			exit
		fi
		;;
	03)
		if [ "$1" != "obsvx1" ]; then
			echo "ERROR${LINENO}"
			exit
		fi
		;;
	esac

	case ${LIST[0]} in
	01)
		case "${LIST[1]}${LIST[2]}" in
		0101)
			echo "EHS6"
			exit 7
			;;
		0201|0202|0203|0204)
			echo "U200"
			exit 5
			;;
		0501)
			echo "S710"
			exit 9
			;;
		FFFF|ffff)
			echo "none"
			exit 0
			;;
		*)
			echo "ERROR${LINENO} (${LIST[0]}${LIST[1]}${LIST[2]})"
			exit 8
			;;
		esac
		;;
	02)
		case "${LIST[1]}${LIST[2]}" in
		0201|0202|0203|0204)
			echo "U200E"
			exit 6
			;;
		0301)
			echo "KYM11"
			exit 4
			;;
		0401)
			echo "UM04"
			exit 2
			;;
		0501)
			echo "S710E"
			exit 9
			;;
		0601)
			echo "EC45"
			exit 10
			;;
		FFFF|ffff)
			echo "none"
			exit 0
			;;
		*)
			echo "ERROR${LINENO} (${LIST[0]}${LIST[1]}${LIST[2]})"
			exit 8
			;;
		esac
		;;
	03)
		case "${LIST[1]}${LIST[2]}" in
		0201|0202|0203|0204)
			echo "U200E"
			exit 6
			;;
		0301)
			echo "KYM11"
			exit 4
			;;
		0401)
			echo "UM04"
			exit 2
			;;
		0501)
			echo "S710E"
			exit 9
			;;
		0601)
			echo "EC45"
			exit 10
			;;
		FFFF|ffff)
			echo "none"
			exit 0
			;;
		*)
			echo "ERROR${LINENO} (${LIST[0]}${LIST[1]}${LIST[2]})"
			exit 8
			;;
		esac
		;;
	*)
		echo "ERROR${LINENO} (${LIST[0]}${LIST[1]}${LIST[2]})"
		exit 8
		;;
	esac
}

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
obsvx*)
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
obsgem*)
	[ ! -e $GPIOPATH/gpio462 ] && echo 462 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio463 ] && echo 463 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio464 ] && echo 464 > $GPIOPATH/export
	[ ! -e $GPIOPATH/gpio465 ] && echo 465 > $GPIOPATH/export

	echo in > $GPIOPATH/gpio462/direction
	echo in > $GPIOPATH/gpio463/direction
	echo in > $GPIOPATH/gpio464/direction
	echo in > $GPIOPATH/gpio465/direction
	echo 1 > $GPIOPATH/gpio462/active_low
	echo 1 > $GPIOPATH/gpio463/active_low
	echo 1 > $GPIOPATH/gpio464/active_low
	echo 0 > $GPIOPATH/gpio465/active_low

	id0=`cat $GPIOPATH/gpio462/value`
	id1=`cat $GPIOPATH/gpio463/value`
	id2=`cat $GPIOPATH/gpio464/value`
	id3=`cat $GPIOPATH/gpio465/value`

	echo 462 > $GPIOPATH/unexport
	echo 463 > $GPIOPATH/unexport
	echo 464 > $GPIOPATH/unexport
	echo 465 > $GPIOPATH/unexport
	;;
*)
	id0="F"
	id1="F"
	id2="F"
	;;
esac

VER=`cut -c 1 /etc/debian_version`

if [ "$MODEL" == "obsvx1" -o "$MODEL" == "obsvx2" ]; then
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
	111)
		_call_obsutil "obsvx1"
		exit 10
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
elif [ "$MODEL" == "obsbx1" -a "$VER" == "7" ]; then
	case "$id2$id1$id0" in
	000)            # EX1 none or BX0
		echo "none"
		exit 0
		;;
	010)            # EX1 UM04-KO
		echo "UM04"
		exit 2
		;;
	100)            # EX1 KYM11/12
		echo "KYM11"
		exit 4
		;;
	110)            # EX1 U200/U270
		echo "U200E"
		exit 6
		;;
	101)            # BX3 U200/U270
		echo "U200"
		exit 5
		;;
	111)            # BX1 EHS6
		echo "EHS6"
		exit 7
		;;
	*)
		echo "ERROR ($id2$id1$id0)"
		exit 8
		;;
	esac
elif [ "$MODEL" == "obsbx1" ]; then
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
	0|8)
		echo "none"
		exit 0
		;;
	48)
		echo "S710E"
		exit 9
		;;
	18)
		echo "S710"
		exit 9
		;;
	68)
		_call_obsutil "obsbx1"
		exit 10
		;;
	*)
		echo "ERROR ($dipsw)"
		exit 8
		;;
	esac
elif [ "$MODEL" == "obsgem1" ]; then
	case "$id3$id2$id1$id0" in
	0111)			# EX1 none or BX0
		echo "none"
		exit 0
		;;
	0101)			# EX1 UM04-KO
		echo "UM04"
		exit 2
		;;
	0011)			# EX1 KYM11/12
		echo "KYM11"
		exit 4
		;;
	0001)			# EX1 U200/U270
		echo "U200E"
		exit 6
		;;
	0010)			# BX3 U200/U270
		echo "U200"
		exit 5
		;;
	0000)			# BX1 EHS6
		echo "EHS6"
		exit 7
		;;
	0100)			# BX3 U200/U270
		echo "ERROR ($id3$id2$id1$id0)"
		exit 8
		;;
	0110)
		echo "S710"
		exit 10
		;;
	1011)
		echo "S710E"
		exit 9
		;;
	*)
		echo "ERROR ($id3$id2$id1$id0)"
		exit 8
		;;
	esac
fi
