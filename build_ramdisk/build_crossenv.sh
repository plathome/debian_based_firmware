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

# Install packages of Debian.

packages="build-essential uboot-mkimage libncurses5-dev debootstrap vim qemu-user-static emdebian-archive-keyring"

host_debian_version=`cat /etc/debian_version`

case $host_debian_version in
	6.*) packages+=" xz-lzma" ;;
esac

apt-get update 
apt-get install $packages

# Install packages of Emdebian.

case $host_debian_version in
	6.*)
		code_name=squeeze
		if [ "$TARGET" == "obs600" ];then
			gcc_version=4.3
		else
			gcc_version=4.4
		fi
	;;
	7.*)
		code_name=wheezy
		gcc_version=4.7
	;;
	*) exit 1 ;;
esac

packages="gcc-${gcc_version}-${KERN_ARCH}-linux-gnu${ABI}"

if [ "$TARGET" == "obs600" ]; then
	xbinutils_version=2.20.1-16
	xgcc_version=4.3.5-4
	pkg4gomp1="libc6-powerpc-cross libgcc1-powerpc-cross"
	packages+=" libc-dev-bin-powerpc-cross libc6-dev-powerpc-cross linux-libc-dev-powerpc-cross zlib1g-powerpc-cross zlib1g-dev-powerpc-cross"
else
	case $host_debian_version in
		7.*)
			xbinutils_version=2.22-7.1
			xgcc_version=4.7.2-4
		;;
		6.*)
			xbinutils_version=2.20.1-16
			xgcc_version=4.4.5-8
		;;
	esac
	pkg4gomp1="libc6-armel-cross libgcc1-armel-cross"
	packages+=" libc-dev-bin-armel-cross libc6-dev-armel-cross linux-libc-dev-armel-cross zlib1g-armel-cross zlib1g-dev-armel-cross"
fi

case $(uname -m) in
	x86_64) deb_arch=amd64 ;;
	i686) deb_arch=i386 ;;
	*) exit 1 ;;
esac

cat <<_EOF >/etc/apt/sources.list.d/emdebian.list 
deb http://www.emdebian.org/debian ${code_name} main
_EOF

apt-get update 

fetch_install ()
{
# fetch_install: fetch and install an package
# args: $1=URL of package; $2=name of package

name=$(echo $2 | sed 's/_[0-9].*$//')
status=$(dpkg --get-selections $name | cut -f 2- | sed 's/\t*//')
if [ "$status" != "install" ]; then
	if [ ! -d ${TMPDIR} ]; then
		mkdir -p ${TMPDIR}
	fi
	wget -O ${TMPDIR}/$2 $1/$2
	dpkg -i ${TMPDIR}/$2
	rm -f ${TMPDIR}/$2
fi
}

fetch_install http://www.emdebian.org/debian/pool/main/b/binutils/ binutils-${KERN_ARCH}-linux-gnu${ABI}_${xbinutils_version}_${deb_arch}.deb
fetch_install http://www.emdebian.org/debian/pool/main/g/gcc-${gcc_version}/ gcc-${gcc_version}-${KERN_ARCH}-linux-gnu${ABI}-base_${xgcc_version}_${deb_arch}.deb
fetch_install http://www.emdebian.org/debian/pool/main/g/gcc-${gcc_version}/ cpp-${gcc_version}-${KERN_ARCH}-linux-gnu${ABI}_${xgcc_version}_${deb_arch}.deb
apt-get install $pkg4gomp1
if [ "$TARGET" == "obs600" ]; then
	fetch_install http://www.emdebian.org/debian/pool/main/g/gcc-${gcc_version}/ libgomp1-powerpc-cross_${xgcc_version}_all.deb
else
	fetch_install http://www.emdebian.org/debian/pool/main/g/gcc-${gcc_version}/ libgomp1-armel-cross_${xgcc_version}_all.deb
fi
apt-get install $packages

update-alternatives --install /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-gcc ${KERN_ARCH}-linux-gnu${ABI}-gcc /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-gcc-${gcc_version} 255
update-alternatives --install /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-cpp ${KERN_ARCH}-linux-gnu${ABI}-cpp /usr/bin/${KERN_ARCH}-linux-gnu${ABI}-cpp-${gcc_version} 255

update-alternatives --set editor /usr/bin/vim.basic

if [ "$TARGET" == "obs600" ]; then
	dpkg -P qemu-user-static
	fetch_install http://ftp.jp.debian.org/debian/pool/main/q/qemu qemu-user-static_1.1.2+dfsg-6a_i386.deb
fi
