#!/bin/bash

. `dirname $0`/config.sh

[ "${ARCH}" != "powerpc" ] && exit

chroot ${DISTDIR} /usr/bin/aptitude purge -y libc6-ppc64
