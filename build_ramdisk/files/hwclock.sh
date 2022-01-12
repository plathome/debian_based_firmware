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
case $MODEL in
obsbx1) HWCLOCK=/usr/local/bin/hwclock ;;
*) HWCLOCK=/sbin/hwclock ;;
esac


usage()
{
	echo -e "--systohc\t: set the hardware clock from the system time"
	echo -e "--hctosys\t: set the system time from the hardware clock"
	echo -e "--alarm2\t: print the alarm time by using INT2"
	echo -e "--alarm2 HHMMw\t: set the alarm time by using INT2"
	echo -e "\t HHMM"
	echo -e "\t MM"
	echo -e "\t\tH = Hours M = Minutes"
	echo -e "\t\tw = 0:Sunday, 1:Monday...6:Saturday"
	echo -e "--clearint2\t: clear INT2 flag"
	if [ "$1" == "--help" ]; then
		echo -e ""
		echo -e "--alarm1\t: print the alarm time by using INT1(DEBUG)"
		echo -e "--alarm1 HHMMw: set the alarm time by using INT1(DEBUG)"
		echo -e "--clearint1\t: clear INT1 flag(DEBUG)"
		echo -e "--init\t\t: reset and initialize the RTC(DEBUG)"
		echo -e "--status\t: print the status of RTC(DEBUG)"
		echo -e "--check\t\t: check RTC(DEBUG)"
		echo -e "--rtc\t\t: support /etc/init.d/hwclock.sh(DEBUG)"
		echo -e "--help\t\t: this messages"
	fi
	return 0
}

do_obs_hwclock()
{
	case "$1" in
	"")
		obs-hwclock --hctosys
	;;
	--alarm*|--clear*|--init|--status|--check)
		obs-hwclock $@
	;;
	--systohc|--rtc*)
		obs-hwclock --systohc `date +'%s'`
	;;
	--hctosys)
		date -s "`obs-hwclock --hctosys`"
	;;
	--help)
		usage $@
	;;
	*)
		usage
	;;
	esac
}

obs-hwclock --check
if [ $? == 0 ]; then	# BX0 or EX1 rev2
	do_obs_hwclock $@
	if [ $? != 0 ]; then
		usage
	fi
elif [ $? == 127 ]; then
	hwclock $@
else
	case $@ in
	--alarm*|--clear*|--init|--status|--check)
		$HWCLOCK --help
	;;
	*)
		$HWCLOCK $@	
	;;
	esac
fi

exit $?
