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

#
# check host machine
#
if ! grep -q "^12" /etc/debian_version ; then
	echo "This machine is not Debian 12"
	echo "Abort!!"
#	exit 1
fi

#
# add architecture
#
dpkg --add-architecture i386
dpkg --add-architecture arm64
dpkg --add-architecture armhf
#dpkg --add-architecture armel
apt update 

#
# host machine development tools
#
packages="build-essential libncurses5-dev debootstrap qemu-user-static bc gcc-multilib xz-utils dosfstools libnl-3-dev libssl-dev libusb-dev libasound2-dev libi2c-dev libusb-1.0-0 libusb-1.0-0-dev pkg-config gcc-multilib g++-multilib flex bison libelf-dev rsync wget libfdt-dev lzop u-boot-tools binfmt-support"
apt -y install $packages

#
# i386 development tools
#
packages="libnl-3-dev:i386 libssl-dev:i386 libusb-dev:i386 libnl-genl-3-dev:i386 libglib2.0-dev:i386 libbluetooth-dev:i386 libasound2-dev:i386 libusb-1.0-0-dev:i386 libi2c-dev:i386"
apt -y install $packages

#
# armel development tools
#
#packages="gcc-arm-linux-gnueabi zlib1g-dev:armel libusb-dev:armel "
#apt -y install $packages

#
# armhf development tools
#
packages="gcc-arm-linux-gnueabihf zlib1g-dev:armhf libusb-dev:armhf "
apt -y install $packages
 
#
# arm64 development tools
#
packages="gcc-aarch64-linux-gnu g++-aarch64-linux-gnu zlib1g-dev:arm64 libusb-dev:arm64 libasound2-dev:arm64 libusb-1.0-0-dev:arm64 libi2c0:arm64"
apt -y install $packages
