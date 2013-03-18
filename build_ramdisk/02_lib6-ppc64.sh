#!/bin/bash

. `dirname $0`/config.sh

[ "${ARCH}" != "powerpc" ] && exit

chroot ${SQUEEZEDIR} /usr/bin/aptitude purge -y libc6-ppc64
