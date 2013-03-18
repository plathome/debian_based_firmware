#!/bin/bash

. `dirname $0`/config.sh

cp ${FILESDIR}/flashcfg.sh ${SQUEEZEDIR}/usr/sbin/flashcfg
chmod 555 ${SQUEEZEDIR}/usr/sbin/flashcfg

cp ${FILESDIR}/usbreset.sh ${SQUEEZEDIR}/usr/sbin/usbreset
chmod 555 ${SQUEEZEDIR}/usr/sbin/usbreset

(cd ${FILESDIR}; ./build_obstools.sh $TARGET) || exit 1

for cmd in flashcfg-debian runled pshd; do
	(cd ${FILESDIR}; install -c -s -o root -g root -m 555 $cmd ${SQUEEZEDIR}/usr/sbin/$cmd)
done
