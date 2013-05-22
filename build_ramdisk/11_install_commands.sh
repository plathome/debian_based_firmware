#!/bin/bash

. `dirname $0`/config.sh

cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
chmod 555 ${DISTDIR}/usr/sbin/flashcfg

cp ${FILESDIR}/usbreset.sh ${DISTDIR}/usr/sbin/usbreset
chmod 555 ${DISTDIR}/usr/sbin/usbreset

(cd ${FILESDIR}; ./build_obstools.sh $TARGET) || exit 1

for cmd in flashcfg-debian runled pshd; do
	(cd ${FILESDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
	$STRIP ${DISTDIR}/usr/sbin/$cmd
done
