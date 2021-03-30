#!/bin/bash

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

first_conf=/etc/openblocks/rename/tmp_rename.conf
second_conf=/etc/openblocks/rename/act_rename.conf

if ( which ifrename 2>&1 ) > /dev/null ; then
	if [ -f "${first_conf}" -a -f "${second_conf}" ] ; then
		ifrename -c ${first_conf}
		sleep 0.01
		ifrename -c ${second_conf}
	fi
fi

exit 0
