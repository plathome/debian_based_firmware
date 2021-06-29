#! /bin/bash
#
# Copyright (c) 2013-2021 Plat'Home CO., LTD.
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
case $MODEL in
obsvx*|obsix9)
	# runled
	[ ! -d $GPIOPATH/gpio342 ] && echo 342 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio342/direction
	[ ! -d $GPIOPATH/gpio343 ] && echo 343 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio343/direction
	[ ! -d $GPIOPATH/gpio344 ] && echo 344 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio344/direction

	# pshd
	[ ! -d $GPIOPATH/gpio345 ] && echo 345 > $GPIOPATH/export; \
		echo both > $GPIOPATH/gpio345/edge

	# obsiot-power
	[ ! -d $GPIOPATH/gpio366 ] && echo 366 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio367 ] && echo 367 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio365 ] && echo 365 > $GPIOPATH/export
	;;
obsbx*)
	# runled
	[ ! -d $GPIOPATH/gpio47 ] && echo 47 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio47/direction
	[ ! -d $GPIOPATH/gpio48 ] && echo 48 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio48/direction
	[ ! -d $GPIOPATH/gpio49 ] && echo 49 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio49/direction

	# pshd
	[ ! -d $GPIOPATH/gpio14 ] && echo 14 > $GPIOPATH/export; \
		echo both > $GPIOPATH/gpio14/edge

	# obsiot-power
	[ ! -d $GPIOPATH/gpio40 ] && echo 40 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio41 ] && echo 41 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio42 ] && echo 42 > $GPIOPATH/export
	;;
obsa16)
	# runled
	[ ! -d $GPIOPATH/gpio488 ] && echo 488 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio488/direction
	[ ! -d $GPIOPATH/gpio489 ] && echo 489 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio489/direction
	[ ! -d $GPIOPATH/gpio490 ] && echo 490 > $GPIOPATH/export; \
		echo out > $GPIOPATH/gpio490/direction

	# pshd
	[ ! -d $GPIOPATH/gpio466 ] && echo 466 > $GPIOPATH/export; \
		echo both > $GPIOPATH/gpio466/edge

	# obsiot-power
	[ ! -d $GPIOPATH/gpio500 ] && echo 500 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio503 ] && echo 503 > $GPIOPATH/export
	[ ! -d $GPIOPATH/gpio504 ] && echo 504 > $GPIOPATH/export
	;;
*)
	exit 1
	;;
esac

export DEBIAN_FRONTEND=noninteractive
dpkg-reconfigure openssh-server

exit 0
