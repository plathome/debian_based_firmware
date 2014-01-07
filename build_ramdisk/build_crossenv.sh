#!/bin/bash
#
# Copyright (c) 2013, 2014 Plat'Home CO., LTD.
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

packages="build-essential uboot-mkimage libncurses5-dev debootstrap binutils-${KERN_ARCH}-linux-gnu${ABI} vim qemu-user-static"

if [ "$TARGET" == "obs600" ]; then
	packages+=" libc6-powerpc-cross libc6-dev-powerpc-cross zlib1g-dev-powerpc-cross"
else
	packages+=" libc6-armel-cross libc6-dev-armel-cross zlib1g-dev-armel-cross"
fi

host_debian_version=`cat /etc/debian_version`

case $host_debian_version in
	6.*)
		code_name=squeeze
		if [ "$TARGET" == "obs600" ];then
			gcc_version=4.3
		else
			gcc_version=4.4
		fi
		packages+=" gcc-${gcc_version}-${KERN_ARCH}-linux-gnu${ABI} xz-lzma"
	;;
	7.*)
		code_name=wheezy
		gcc_version=4.7
		packages+=" gcc-${gcc_version}-${KERN_ARCH}-linux-gnu${ABI}"
	;;
	*) exit 1 ;;
esac

apt-get update 
apt-get install emdebian-archive-keyring

cat <<_EOF >/etc/apt/sources.list.d/emdebian.list 
deb http://www.emdebian.org/debian ${code_name} main
_EOF

apt-get update 
apt-get install $packages

update-alternatives --install /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-gcc ${KERN_ARCH}-linux-gnu${ABI}-gcc /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-gcc-${gcc_version} 255
update-alternatives --install /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-cpp ${KERN_ARCH}-linux-gnu${ABI}-cpp /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-cpp-${gcc_version} 255

update-alternatives --set editor /usr/bin/vim.basic

if [ "$TARGET" == "obs600" ]; then
	PKGURL="http://ftp.jp.debian.org/debian/pool/main/q/qemu"
	PKGNAME="qemu-user-static_1.1.2+dfsg-6a_i386.deb"
	dpkg -r qemu-user-static
	wget -O /tmp/${PKGNAME} ${PKGURL}/${PKGNAME}
	dpkg -i /tmp/${PKGNAME}
	rm -f /tmp/${PKGNAME}
fi
