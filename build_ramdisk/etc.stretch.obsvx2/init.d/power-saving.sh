#! /bin/bash
### BEGIN INIT INFO
# Provides:          power-saving
# Required-Start:    $local_fs $syslog $remote_fs
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:
# Short-Description: Power saving for OBSVX1
# Description:       
### END INIT INFO
#
# Copyright (c) 2013-2018 Plat'Home CO., LTD.
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

# Edit here
Ethernet=yes
RS485=yes
USB_SPI=yes

# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script
PATH=/usr/local/sbin:/sbin:/usr/sbin:/bin:/usr/bin
DESC="Power saving for OBSVX1"
NAME=power-saving.sh
SCRIPTNAME=/etc/init.d/$NAME

# Read configuration variable file if it is present
[ -r /etc/default/openblocks ] && . /etc/default/openblocks

PM_Ethernet="/sys/devices/pci0000:00/0000:00:1c.3/0000:04:00.0"
PM_RS485="/sys/devices/pci0000:00/0000:00:14.0/usb1/1-4/1-4.1"
PM_USB_SPI="/sys/devices/pci0000:00/0000:00:14.0/usb1/1-4/1-4.2"

case "$1" in
  start)
	# Ethernet
	if [ "$Ethernet" == "yes" ]; then
		if [ -w ${PM_Ethernet}/power/control ]; then
			echo auto > ${PM_Ethernet}/power/control
		else
			logger -p user.notice -t $NAME "Cannot power saving of Ethernet"
		fi
	fi
	# RS485
	if [ "$RS485" == "yes" ]; then
		if [ -w ${PM_RS485}/power/control ]; then
			echo auto > ${PM_RS485}/power/control
		else
			logger -p user.notice -t $NAME "Cannot power saving of RS485"
		fi
	fi
	# USB-SPI
	if [ "$USB_SPI" == "yes" ]; then
		if [ -w ${PM_USB_SPI}/power/control ]; then 
			echo auto > ${PM_USB_SPI}/power/control
		else
			logger -p user.notice -t $NAME "Cannot power saving of USB-SPI"
		fi
	fi
	;;
  stop)
	;;
  *)
	echo "Usage: $SCRIPTNAME {start|stop}" >&2
	exit 2
	;;
esac

