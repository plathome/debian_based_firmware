#!/bin/bash

. `dirname $0`/config.sh

(cd ${FILESDIR};sort etc/exclude.list > /tmp/exclude.list.new; mv -f /tmp/exclude.list.new etc/exclude.list)

(cd ${FILESDIR};tar --exclude=CVS -cvf - etc)| tar xvf - -C ${DISTDIR}
(cd ${FILESDIR_ADD};tar --exclude=CVS -cvf - etc)| tar xvf - -C ${DISTDIR}

chmod 755 ${DISTDIR}/etc/init.d/openblocks-setup
(cd ${DISTDIR}/etc/rcS.d; ln -sf ../init.d/openblocks-setup S01openblocks-setup)
(cd ${DISTDIR}/etc/rc6.d; ln -sf ../init.d/openblocks-setup S39openblocks-setup)
(cd ${DISTDIR}/etc/rc0.d; ln -sf ../init.d/openblocks-setup K07openblocks-setup)

chmod 755 ${DISTDIR}/etc/init.d/pshd
(cd ${DISTDIR}/etc/rc0.d; ln -sf ../init.d/pshd K01pshd)
(cd ${DISTDIR}/etc/rc6.d; ln -sf ../init.d/pshd K01pshd)
(cd ${DISTDIR}/etc/rc2.d; ln -sf ../init.d/pshd S99pshd)

chmod 755 ${DISTDIR}/etc/init.d/runled
(cd ${DISTDIR}/etc/rc0.d; ln -sf ../init.d/runled K01runled)
(cd ${DISTDIR}/etc/rc6.d; ln -sf ../init.d/runled K01runled)
(cd ${DISTDIR}/etc/rc2.d; ln -sf ../init.d/runled S99runled)
