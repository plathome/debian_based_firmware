#!/bin/bash
#
# Copyright (c) 2013-2016 Plat'Home CO., LTD.
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

trap '$ATCMD POFF;rm -f $PID_FILE' INT QUIT TERM

DEBUG=no
[ "$1" = "DEBUG" ] && DEBUG=yes
ATCMD=/usr/sbin/atcmd

PID_FILE="/var/run/chksignal.sh.pid"
if [ -f $PID_FILE ]; then
	echo "$0 already running."
	exit 1
else
	echo $$ > $PID_FILE
fi

RUNLED_CONFIG="/tmp/.runled"
if [ ! -f $RUNLED_CONFIG ]; then
	echo -e "1\n1\n0" > $RUNLED_CONFIG
fi

$ATCMD PON
ccid=`$ATCMD CCID`
if [ "$ccid" = "" ]; then
	echo -e "1\n1\n2" > $RUNLED_CONFIG
	$ATCMD POFF
	rm -f $PID_FILE
	exit 1
fi

sleep 20				# wait for SMONI
for i in {0..360}
do
	val=`$ATCMD SMONI | sed -e "s|-||"`
	echo -e "3\n2\n0" > $RUNLED_CONFIG
	[ "$DEBUG" != no ] && echo "smoni=$val"
	if [ "$val" = "" ]; then
		echo -e "3\n2\n5" > $RUNLED_CONFIG
		[ "$DEBUG" != no ] && echo "purple"
	elif [ $val -le 87 ]; then
		echo -e "3\n2\n7" > $RUNLED_CONFIG
		[ "$DEBUG" != no ] && echo "white"
	elif [ $val -ge 88 -a $val -le 106 ]; then
		echo -e "3\n2\n6" > $RUNLED_CONFIG
		[ "$DEBUG" != no ] && echo "aqua"
	elif [ $val -ge 107 -a $val -le 129 ]; then
		echo -e "3\n2\n4" > $RUNLED_CONFIG
		[ "$DEBUG" != no ] && echo "blue"
	else
		echo -e "3\n2\n5" > $RUNLED_CONFIG
		[ "$DEBUG" != no ] && echo "purple"
	fi
done

$ATCMD POFF

rm -f $PID_FILE
