#!/bin/bash
#set -x
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

[ -e "/etc/default/openblocks" ] && . /etc/default/openblocks

case $MODEL in
obsa16*|obsfx0*|obsfx1*|obsgx4*|obsduo) ;;
*) exit 0 ;;
esac

for one in /sys/class/net/*
do
	case "${one}" in
		"/sys/class/net/eth"*)
			IF=`echo $one | sed -e "s|/sys/class/net/||"`
			NO=`echo $IF | sed -e "s|eth||"`
			TMPFILE=/tmp/.${IF}_$$
			if ( /usr/sbin/obs-util -M ${NO} ${TMPFILE} 2>&1 ) > /dev/null ; then
				( /usr/sbin/ip link set dev ${IF} address $(cut -b 1-2 ${TMPFILE}):$(cut -b 3-4 ${TMPFILE}):$(cut -b 5-6 ${TMPFILE}):$(cut -b 7-8 ${TMPFILE}):$(cut -b 9-10 ${TMPFILE}):$(cut -b 11-12 ${TMPFILE}) 2>&1 ) > /dev/null
			fi
			rm -f $TMPFILE
			;;
	esac
done

exit 0
