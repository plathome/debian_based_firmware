#!/bin/bash

. `dirname $0`/config.sh

cp ${FILESDIR}/public_20090828.key ${DISTDIR}/
chroot ${DISTDIR} /usr/bin/apt-key add /public_20090828.key
rm -f ${DISTDIR}/public_20090828.key
