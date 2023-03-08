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

rename_func() {
	if [ -x /usr/sbin/ifrename -a -f ${1} ] ; then
		ifrename -c ${1}
	fi
}

cp_func() {
	if [ ! -s /etc/network/interfaces -a -f ${1} ] ; then
		cp -p ${1} /etc/network/interfaces
	fi
}

case $MODEL in
obshx1*)
	sleep 5

	rules=/etc/udev/rules.d/75-net-description.rules

	net_ifnames=true
	grep -q 'net.ifnames=0' /proc/cmdline && net_ifnames=false

	if [ -L $rules -o $net_ifnames = false ] ; then
		if [ -e /sys/class/net/eth3 ]; then
			rename_func /etc/openblocks/rename/${MODEL}-4port_tmp.conf
			sleep 0.01
			rename_func /etc/openblocks/rename/${MODEL}-4port.conf
			cp_func /etc/network/interfaces-4port
		elif [ -e /sys/class/net/eth2 ]; then
			rename_func /etc/openblocks/rename/${MODEL}-3port_tmp.conf
			sleep 0.01
			rename_func /etc/openblocks/rename/${MODEL}-3port.conf
			cp_func /etc/network/interfaces-3port
		else
			rename_func /etc/openblocks/rename/${MODEL}-2port_tmp.conf
			sleep 0.01
			rename_func /etc/openblocks/rename/${MODEL}-2port.conf
			cp_func /etc/network/interfaces-2port
		fi
	else
		if [ -e /sys/class/net/enp2s0 ]; then
			rename_func /etc/openblocks/rename/${MODEL}-4port.conf
			cp_func /etc/network/interfaces-4port
		elif [ -e /sys/class/net/enp3s0 ]; then
			rename_func /etc/openblocks/rename/${MODEL}-3port.conf
			cp_func /etc/network/interfaces-3port
		else
			rename_func /etc/openblocks/rename/${MODEL}-2port.conf
			cp_func /etc/network/interfaces-2port
		fi
	fi
	;;
obshx2*)
	sleep 5

	rules=/etc/udev/rules.d/75-net-description.rules

	net_ifnames=true
	grep -q 'net.ifnames=0' /proc/cmdline && net_ifnames=false

	if [ -L $rules -o $net_ifnames = false ] ; then
		if [ -e /sys/class/net/eth2 ]; then
			rename_func /etc/openblocks/rename/${MODEL}-3port_tmp.conf
			sleep 0.01
			rename_func /etc/openblocks/rename/${MODEL}-3port.conf
			cp_func /etc/network/interfaces-3port
		else
			rename_func /etc/openblocks/rename/${MODEL}-2port_tmp.conf
			sleep 0.01
			rename_func /etc/openblocks/rename/${MODEL}-2port.conf
			cp_func /etc/network/interfaces-2port
		fi
	else
		if [ -e /sys/class/net/eno1 ]; then
			rename_func /etc/openblocks/rename/${MODEL}-3port.conf
			cp_func /etc/network/interfaces-3port
		else
			rename_func /etc/openblocks/rename/${MODEL}-2port.conf
			cp_func /etc/network/interfaces-2port
		fi
	fi
	;;
obsix9*)
	first_conf=/etc/openblocks/rename/tmp_rename.conf
	second_conf=/etc/openblocks/rename/act_rename.conf

	func_rename ${first_conf}
	sleep 0.01
	func_rename ${second_conf}
	;;
esac

exit 0
