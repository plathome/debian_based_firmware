#!/bin/bash

. `dirname $0`/config.sh

rm -rf ${DISTDIR}/*.deb
cp ${EXTRADEBDIR}/*${ARCH}.deb ${DISTDIR}/

debs=$(cd ${DISTDIR}/; ls -1 *${ARCH}.deb)

chroot ${DISTDIR} dpkg -i $debs
