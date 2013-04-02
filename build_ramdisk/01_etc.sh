#!/bin/bash

. `dirname $0`/config.sh

(cd ${ETCDIR};sort exclude.list > /tmp/exclude.list.new; mv -f /tmp/exclude.list.new exclude.list)

(cd ${ETCDIR};tar --exclude=CVS -cvf - .)| tar xvf - -C ${DISTDIR}/etc/
(cd ${ETCDIR_ADD};tar --exclude=CVS -cvf - .)| tar xvf - -C ${DISTDIR}/etc/


for sh in openblocks-setup pshd runled;do
	chmod 755 ${DISTDIR}/etc/init.d/$sh
	chroot ${DISTDIR} /usr/sbin/update-rc.d $sh defaults
done
