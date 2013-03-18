#!/bin/bash

. `dirname $0`/config.sh

(cd ${PWD}/kernel-image; rm -f dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb)
(cd ${PWD}/kernel-image; ./mkdummy.sh ${KERNEL}-${PATCHLEVEL})

cp -f ${PWD}/kernel-image/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb ${SQUEEZEDIR}/
chroot ${SQUEEZEDIR} dpkg -r kernel-image
chroot ${SQUEEZEDIR} dpkg -i /dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb
rm -f ${SQUEEZEDIR}/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb
