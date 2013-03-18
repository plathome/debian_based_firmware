#!/bin/bash

. `dirname $0`/config.sh

(cd ${PWD}/kernel-image; rm -f dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb)
(cd ${PWD}/kernel-image; ./mkdummy.sh ${KERNEL}-${PATCHLEVEL})

cp -f ${PWD}/kernel-image/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb ${DISTDIR}/
chroot ${DISTDIR} dpkg -r kernel-image
chroot ${DISTDIR} dpkg -i /dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb
rm -f ${DISTDIR}/dummy-kernel-image-${KERNEL}-${PATCHLEVEL}.deb
