#!/bin/bash

. `dirname $0`/config.sh

cp ${FILESDIR}/public_20090828.key ${SQUEEZEDIR}/
chroot ${SQUEEZEDIR} /usr/bin/apt-key add /public_20090828.key
rm -f ${SQUEEZEDIR}/public_20090828.key
