#!/bin/bash

. `dirname $0`/config.sh

if [ "${DIST}" == "wheezy" ] ; then
	chroot ${DISTDIR} /usr/sbin/update-rc.d -f checkroot.sh remove
	chroot ${DISTDIR} /usr/sbin/update-rc.d -f umountroot remove
fi

