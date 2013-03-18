#!/bin/bash

. `dirname $0`/config.sh

cd ${SQUEEZEDIR}

find usr/share/locale/ -maxdepth 1 | grep -Ev "(locale/|locale|ja)$" | xargs rm -rf

rm -rf var/lib/apt/lists/{ftp,security}* var/cache/apt/*.bin var/cache/apt/archives/*.deb root/.bash_history

if [ -f "${SQUEEZEDIR}/usr/share/doc/base-files/README" ] ; then
	mv usr/share/doc usr/share/doc.orig
	mkdir usr/share/doc
	(cd ${SQUEEZEDIR}/usr/share/doc.orig; \
	find . -maxdepth 2 -type f  -name copyright | xargs tar cf - ) | \
	(cd ${SQUEEZEDIR}/usr/share/doc; tar xvf -)
	rm -rf ${SQUEEZEDIR}/usr/share/doc.orig
fi

rm -f etc/ssh/ssh_host*key etc/ssh/ssh_host*key.pub

> etc/resolv.conf
