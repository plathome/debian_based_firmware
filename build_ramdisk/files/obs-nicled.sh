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

RTL8211F_ETH0=/sys/devices/platform/soc@0/30800000.bus/30be0000.ethernet/mdio_bus/30be0000.ethernet-1/30be0000.ethernet-1:01/rtl8211f
RTL8211F_ETH1=/sys/devices/platform/soc@0/30800000.bus/30bf0000.ethernet/mdio_bus/stmmac-1/stmmac-1:01/rtl8211f
LED_VALUE=6f12
EEELED_VALUE=0

case $MODEL in
obsa16*)
	if dmesg | grep -q RTL8211F; then
		echo ${EEELED_VALUE} > ${RTL8211F_ETH0}/rtl8211f_eeelcr
		echo ${EEELED_VALUE} > ${RTL8211F_ETH1}/rtl8211f_eeelcr
		echo ${LED_VALUE} > ${RTL8211F_ETH0}/rtl8211f_lcr
		echo ${LED_VALUE} > ${RTL8211F_ETH1}/rtl8211f_lcr
	fi
	;;
*)
	;;
esac

exit 0
