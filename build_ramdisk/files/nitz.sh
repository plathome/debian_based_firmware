#! /bin/bash
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
GPIOPATH="/sys/class/gpio"
ATCMD="/usr/sbin/atcmd -d /dev/ttyMODEM0"

# Read configuration variable file if it is present
[ -r /etc/default/openblocks ] && . /etc/default/openblocks

set_time_ehs6(){
	# check builtin MODEM
	echo -n "nitz: 3G modem power on : "
	$ATCMD PON
	if [ ! -e /dev/ttyMODEM0 ]; then
		echo "$LINENO: fail"
		exit 1
	fi
	echo "done"

	echo -n "nitz: check SIM card : "
	# check SIM card
	val=`$ATCMD CCID`
	if [ $? == 255 ]; then
		echo "$LINENO: fail"
#		$ATCMD POFF
		exit 1
	fi
	echo "done"

	echo -n "nitz: time synchronization : "
	val=`$ATCMD SIND`
	if [ $? == 255 ]; then
		echo "$LINENO: fail"
#		$ATCMD POFF
		exit 1
	fi
	echo "done"

	# set NITZ to SYSTEM
	date "$val" --utc
	# set SYSTEM to RTC
	hwclock --systohc
#	$ATCMD POFF
}

set_time_u200(){
	# check builtin MODEM
	echo -n "nitz: 3G modem power on : "
	$ATCMD PON
	if [ ! -e /dev/ttyMODEM0 ]; then
		echo "$LINENO: fail"
		exit 1
	fi
	echo "done"

	echo -n "nitz: check SIM card : "
	# check SIM card
	sleep 2
	val=`$ATCMD CCID`
	if [ $? == 255 ]; then
		echo "$LINENO: fail"
#		$ATCMD POFF
		exit 1
	fi
	echo "done"

	echo -n "nitz: time synchronization : "
	val=`$ATCMD CCLK`
	if [ $? == 255 ]; then
		echo "$LINENO: fail"
#		$ATCMD POFF
		exit 1
	fi
	echo "done"

	# set NITZ to SYSTEM
	date "$val"
	# set SYSTEM to RTC
	hwclock --systohc
#	$ATCMD POFF
}

set_time_kym11(){
	echo "done"
}

set_time_s710(){
	echo "done"
}

# check init sw
grep -q 'noflashcfg=1' /proc/cmdline
if [ $? == 0 ]; then
	echo "nitz: push INIT switch"
	exit 0
fi

MODEM=`/usr/sbin/obsiot-modem.sh`

if [ "$MODEM" != "none" -a \( "$MODEL" == "obsvx1" -o "$MODEL" == "obsvx2" \) ]; then
	obsvx1-modem init
	[ "$MODEM" == "S710" ] && obsvx1-modem power low
	atcmd PON
elif [ "$MODEL" == "obsbx1" ]; then
	case $MODEM in
	U200E|UM04)
		echo 200 > $GPIOPATH/export	# 3G modem power
		echo high > $GPIOPATH/gpio200/direction
		echo 202 > $GPIOPATH/export	# 3G reset
		echo high > $GPIOPATH/gpio202/direction
		;;
	EC25)
		echo 200 > $GPIOPATH/export	# 3G modem power
		echo low > $GPIOPATH/gpio200/direction
		echo 202 > $GPIOPATH/export	# 3G reset
		echo high > $GPIOPATH/gpio202/direction
		atcmd PON
		;;
	KYM11|S710E)
		echo 202 > $GPIOPATH/export	# 3G reset
		echo high > $GPIOPATH/gpio202/direction
		echo 200 > $GPIOPATH/export	# 3G modem power
		echo low > $GPIOPATH/gpio200/direction
		if [ ! -e /dev/ttyMODEM0 ]; then
			echo 1 > $GPIOPATH/gpio200/value
			sleep 4
			echo 0 > $GPIOPATH/gpio200/value
			sleep 20
		fi
		;;
	EHS6|S710)
		echo 165 > $GPIOPATH/export	# 3G modem power
		echo low > $GPIOPATH/gpio165/direction
		echo 15 > $GPIOPATH/export	# 3G reset
		echo low > $GPIOPATH/gpio15/direction
		;;
	U200)
		echo 165 > $GPIOPATH/export	# 3G modem power
		echo high > $GPIOPATH/gpio165/direction
		echo 15 > $GPIOPATH/export	# 3G reset
		echo low > $GPIOPATH/gpio15/direction
		;;
	*)
		;;
	esac
fi
case $MODEM in
EHS6)
	set_time_ehs6
	;;
U200|S710)
	set_time_u200
	;;
S710)
	set_time_s710
	;;
U200E|KYM11|UM04|S710E|EC25|none)
	echo "nitz: skipped time synchronization"
	;;
*)
	echo "$LINENO: fail"
	exit 1
	;;
esac

exit 0
