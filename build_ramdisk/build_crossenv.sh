#!/bin/bash
. `dirname $0`/config.sh

packages="build-essential uboot-mkimage libncurses5-dev libc6-armel-cross libc6-dev-armel-cross binutils-arm-linux-gnueabi gcc-4.7-arm-linux-gnueabi zlib1g-dev-armel-cross vim qemu-user-static"

cat <<_EOF >/etc/apt/sources.list.d/emdebian.list 
deb http://www.emdebian.org/debian wheezy main
_EOF

apt-get update 
apt-get install $packages

update-alternatives --install /usr/bin/arm-linux-gnueabi-gcc arm-linux-gnueabi-gcc /usr/bin/arm-linux-gnueabi-gcc-4.7 255
update-alternatives --install /usr/bin/arm-linux-gnueabi-cpp arm-linux-gnueabi-cpp /usr/bin/arm-linux-gnueabi-cpp-4.7 255

update-alternatives --set editor /usr/bin/vim.basic
