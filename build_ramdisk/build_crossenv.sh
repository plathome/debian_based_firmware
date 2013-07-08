#!/bin/bash
. `dirname $0`/config.sh

packages="build-essential uboot-mkimage libncurses5-dev debootstrap libc6-armel-cross libc6-dev-armel-cross binutils-arm-linux-gnueabi zlib1g-dev-armel-cross vim qemu-user-static"

host_debian_version=`cat /etc/debian_version`

case $host_debian_version in
	6.*)
		code_name=squeeze
		gcc_version=4.4
		packages="$packages gcc-${gcc_version}-arm-linux-gnueabi xz-lzma"
	;;
	7.*)
		code_name=wheezy
		gcc_version=4.7
		packages="$packages gcc-${gcc_version}-arm-linux-gnueabi"
	;;
esac

cat <<_EOF >/etc/apt/sources.list.d/emdebian.list 
deb http://www.emdebian.org/debian ${code_name} main
_EOF

apt-get update 
apt-get install $packages

update-alternatives --install /usr/bin/arm-linux-gnueabi-gcc arm-linux-gnueabi-gcc /usr/bin/arm-linux-gnueabi-gcc-${gcc_version} 255
update-alternatives --install /usr/bin/arm-linux-gnueabi-cpp arm-linux-gnueabi-cpp /usr/bin/arm-linux-gnueabi-cpp-${gcc_version} 255

update-alternatives --set editor /usr/bin/vim.basic
