#!/bin/bash
#
# Copyright (c) 2013-2022 Plat'Home CO., LTD.
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

[ -f /etc/default/openblocks ] && . /etc/default/openblocks
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

case $MODEL in
obshx*)
	if [ -e /sys/class/net/enp1s0 ]; then
		ifrename -c /etc/network/iftab-4port
		cp -a /etc/network/interfaces-4port /etc/network/interfaces
	elif [ -e /sys/class/net/enp5s0 ]; then
		ifrename -c /etc/network/iftab-3port
		cp -a /etc/network/interfaces-3port /etc/network/interfaces
	elif [ -e /sys/class/net/enp4s0 ]; then
		sleep 5
		if [ -e /sys/class/net/eno1 ]; then
			ifrename -c /etc/network/iftab-hx2-3port
			cp -a /etc/network/interfaces-3port /etc/network/interfaces
		else
			ifrename -c /etc/network/iftab-2port
			cp -a /etc/network/interfaces-2port /etc/network/interfaces
		fi
	else
		sleep 5
		ifrename -c /etc/network/iftab-hx2-2port
		cp -a /etc/network/interfaces-2port /etc/network/interfaces
	fi
	;;
*)
	first_conf=/etc/openblocks/rename/tmp_rename.conf
	second_conf=/etc/openblocks/rename/act_rename.conf

	if ( which ifrename 2>&1 ) > /dev/null ; then
		if [ -f "${first_conf}" -a -f "${second_conf}" ] ; then
			ifrename -c ${first_conf}
			sleep 0.01
			ifrename -c ${second_conf}
		fi
	fi
	;;
esac

exit 0
