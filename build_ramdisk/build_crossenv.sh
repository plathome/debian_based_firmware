#!/bin/bash
. `dirname $0`/config.sh

packages="build-essential uboot-mkimage libncurses5-dev debootstrap libc6-${ARCH}-cross libc6-dev-${ARCH}-cross binutils-${KERN_ARCH}-linux-gnu${ABI} zlib1g-dev-${ARCH}-cross vim qemu-user-static"

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
