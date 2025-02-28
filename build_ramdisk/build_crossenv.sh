#!/bin/bash
#
# Copyright (c) 2013-2022 Plat'Home CO., LTD.
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

dpkg --add-architecture i386
dpkg --add-architecture armhf
dpkg --add-architecture armel
dpkg --add-architecture powerpc

apt-key list | grep -q "Emdebian Toolchain Archive"
if [ $? == 1 ]; then
	wget -q -O - http://emdebian.org/tools/debian/emdebian-toolchain-archive.key | apt-key add -
fi

packages="build-essential u-boot-tools libncurses5-dev debootstrap qemu-user-static bc gcc-multilib lzma dosfstools"

cat <<_EOF >/etc/apt/sources.list.d/emdebian.list 
deb http://www.emdebian.org/tools/debian jessie main
_EOF

apt-get update 
apt-get -y install $packages

packages="libnl-3-dev libssl-dev libusb-dev libasound2-dev libi2c-dev libusb-1.0-0 libusb-1.0-0-dev pkg-config"

apt-get -y install $packages

packages="libnl-3-dev:i386 libssl-dev:i386 libusb-dev:i386 zlib1g-dev:armel zlib1g-dev:armhf zlib1g-dev:powerpc libnl-genl-3-dev:i386 libglib2.0-dev:i386 libbluetooth-dev:i386 libasound2-dev:i386"
#packages+="libusb-dev:armel libusb-dev:armhf libusb-dev:powerpc "

apt-get -y install $packages

packages="crossbuild-essential-armel crossbuild-essential-armhf crossbuild-essential-powerpc"

apt-get -y install $packages
