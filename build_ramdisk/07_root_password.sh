#!/bin/bash
#
# Copyright (c) 2013-2023 Plat'Home CO., LTD.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY PLAT'HOME CO., LTD. AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL PLAT'HOME CO., LTD. AND CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

. `dirname $0`/config.sh

pw='$1$afC9J.v6$Dkq.k8sVRq7n0Py5eWWAp1'
pw_ix9r='$6$Eq7045XjAMxZ3AQo$N471tuPsYCcOJuCEduSo7fv8WyOp96qBwg6dRA7aJtapVoFIc0uiWmQD5Ibs9l4RPsa3R6VrenJnogxp5n9Il.'
pw_a16='$y$j9T$suL2ZomyH7.hwZX0M4khs1$J1CilnhxtR8hBK3FpS4/zTyXnZ497ukKi1f7RNuEXF5'
pw_hx1='$y$j9T$dIV1tf8FpipQuStvw4mkS/$0XNXv0D/yGtjKw.se6EUlFeid8OJJe4jrKQzTXEBFs8'
pw_hx2='$y$j9T$cIZ4iyeCc0UoDXHENwRVT.$UT21DVGCEaGzFjfwfuxdXfoZ.fbnQVvOuyZFvSi.9Z1:19425'

case $TARGET in
obsix9r)
	chroot ${DISTDIR} /usr/sbin/usermod -p $pw_ix9r root
	;;
obsa16*)
	chroot ${DISTDIR} /usr/sbin/usermod -p $pw_a16 root
	;;
obshx1*)
	chroot ${DISTDIR} /usr/sbin/usermod -p $pw_hx1 root
	;;
obshx2*)
	chroot ${DISTDIR} /usr/sbin/usermod -p $pw root
	;;
*)
	chroot ${DISTDIR} /usr/sbin/usermod -p $pw root
esac

# add "dialout" for tty device
chroot ${DISTDIR} /usr/sbin/adduser root dialout

if [ "$DIST" == "stretch" ]; then
	case $TARGET in
	obsbx*)
		# group
		if ! grep -q "^net_bt_admin:" ${DISTDIR}/etc/group; then
			echo "net_bt_admin:x:3001:root" >> ${DISTDIR}/etc/group
		fi
		if ! grep -q "^net_bt:" ${DISTDIR}/etc/group; then
			echo "net_bt:x:3002:root" >> ${DISTDIR}/etc/group
		fi
		if ! grep -q "^inet:" ${DISTDIR}/etc/group; then
			echo "inet:x:3003:root" >> ${DISTDIR}/etc/group
		fi
		if ! grep -q "^net_raw:" ${DISTDIR}/etc/group; then
			echo "net_raw:x:3004:root" >> ${DISTDIR}/etc/group
		fi
		if ! grep -q "^net_admin:" ${DISTDIR}/etc/group; then
			echo "net_admin:x:3005:root" >> ${DISTDIR}/etc/group
		fi
		if ! grep -q "^net_bw_stats:" ${DISTDIR}/etc/group; then
			echo "net_bw_stats:x:3006:root" >> ${DISTDIR}/etc/group
		fi
		if ! grep -q "^net_bw_acct:" ${DISTDIR}/etc/group; then
			echo "net_bw_acct:x:3007:root" >> ${DISTDIR}/etc/group
		fi
		# passed
		sed -e "s|_apt:x:104:65534:|_apt:x:104:3003:|" < ${DISTDIR}/etc/passwd > /tmp/passwd.new
		mv -f /tmp/passwd.new ${DISTDIR}/etc/passwd
		;;
	esac
fi

