#!/bin/bash

. `dirname $0`/config.sh

(cd ${ETCDIR};sort exclude.list > /tmp/exclude.list.new; mv -f /tmp/exclude.list.new exclude.list)

(cd ${ETCDIR};tar --exclude=CVS -cvf - .)| tar xvf - -C ${DISTDIR}/etc/
(cd ${ETCDIR_ADD};tar --exclude=CVS -cvf - .)| tar xvf - -C ${DISTDIR}/etc/

(cd ${DISTDIR}/etc; (cd ${ETCDIR};find . -type f) | xargs chown root:root ${DISTDIR}/etc)

(cd ${DISTDIR}/etc; (cd ${ETCDIR_ADD};find . -type f) | xargs chown root:root ${DISTDIR}/etc)

for sh in openblocks-setup pshd runled ;do
	chmod 755 ${DISTDIR}/etc/init.d/$sh
	chroot ${DISTDIR} /usr/sbin/update-rc.d -f $sh remove
	case $sh in
	openblocks-setup)
		chroot ${DISTDIR} /usr/sbin/update-rc.d $sh start 1 S . stop 1 0 6 .
		;;
	*)
		chroot ${DISTDIR} /usr/sbin/update-rc.d $sh defaults
		;;
	esac
done

touch ${DISTDIR}/etc/init.d/.legacy-bootordering

printf "0.0 0 0.0\n0\nUTC\n" > ${DISTDIR}/etc/adjtime

if [ -f ${DISTDIR}/etc/modules ]; then
	if grep -q "^ipv6" ${DISTDIR}/etc/modules
	then
		echo "/etc/modules: The line, ipv6, exists"
	else
		echo "ipv6" >> ${DISTDIR}/etc/modules
	fi
fi
