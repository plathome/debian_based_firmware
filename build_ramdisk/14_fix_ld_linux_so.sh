#!/bin/bash

. `dirname $0`/config.sh

if [ "$ARCH" == "armhf" ]  && [ "${DIST}" == "wheezy" ] ; then
	ln -sf /lib/ld-linux-armhf.so.3 ${DISTDIR}/lib/ld-linux.so.3
fi

