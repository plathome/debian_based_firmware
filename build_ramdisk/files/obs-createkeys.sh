#!/bin/bash
#
# Copyright (c) 2013-2021 Plat'Home CO., LTD.
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

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

sshd_get_config_option() {
	option="$1"

	[ -f /etc/ssh/sshd_config ] || return

	# TODO: actually only one '=' allowed after option
	perl -lne 's/\s+/ /g; print if s/^\s*'"$option"'[[:space:]=]+//i' \
	   /etc/ssh/sshd_config
}

sshd_host_keys_required() {
	hostkeys="$(sshd_get_config_option HostKey)"
	if [ "$hostkeys" ]; then
		echo "$hostkeys"
	else
		# No HostKey directives at all, so the server picks some
		# defaults depending on the setting of Protocol.
		protocol="$(sshd_get_config_option Protocol)"
		[ "$protocol" ] || protocol=1,2
		if echo "$protocol" | grep 2 >/dev/null; then
			echo /etc/ssh/ssh_host_rsa_key
			echo /etc/ssh/ssh_host_dsa_key
			echo /etc/ssh/ssh_host_ecdsa_key
			echo /etc/ssh/ssh_host_ed25519_key
		fi
	fi
}

sshd_create_key() {
	msg="$1"
	shift
	hostkeys="$1"
	shift
	file="$1"
	shift

	if echo "$hostkeys" | grep -x "$file" >/dev/null && \
	   [ ! -f "$file" ] ; then
		echo -n $msg
		ssh-keygen -q -f "$file" -N '' "$@"
		echo
		if type restorecon >/dev/null 2>&1; then
			restorecon "$file.pub"
		fi
	fi
}


sshd_create_keys() {
	hostkeys="$(sshd_host_keys_required)"

	sshd_create_key "Creating SSH2 RSA key; this may take some time ..." \
		"$hostkeys" /etc/ssh/ssh_host_rsa_key -t rsa
	sshd_create_key "Creating SSH2 DSA key; this may take some time ..." \
		"$hostkeys" /etc/ssh/ssh_host_dsa_key -t dsa
	sshd_create_key "Creating SSH2 ECDSA key; this may take some time ..." \
		"$hostkeys" /etc/ssh/ssh_host_ecdsa_key -t ecdsa
	sshd_create_key "Creating SSH2 ED25519 key; this may take some time ..." \
		"$hostkeys" /etc/ssh/ssh_host_ed25519_key -t ed25519
}

# create ssh keys for openssh-server
if [ -x /usr/sbin/sshd ] ; then
	sshd_create_keys
fi

exit 0
