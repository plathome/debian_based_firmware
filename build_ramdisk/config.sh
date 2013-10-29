
DIST=${DIST:=wheezy}

TARGET=${TARGET:=obsax3}

COMPRESS=${COMPRESS:=lzma}

COMPRESS_EXT=${COMPRESS_EXT:=lzma}

export TARGET DIST

if [ "$TARGET" == "obs600" ]; then
       QEMU_BIN=qemu-ppc-static
       GCCVER=4.3
       ABI=""
else
 QEMU_BIN=qemu-arm-static
       GCCVER=4.7
       ABI=eabi
fi

case ${DIST} in
wheezy)
	KERNEL=3.2.40
	case ${TARGET} in
	obsax3)
		ISOFILE=debian-7.1.0-armhf-DVD-1.iso
		RAMDISK_SIZE=160
		PATCHLEVEL=3
		ARCH=armhf
	;;
	obsa6)
		ISOFILE=debian-7.1.0-armel-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=2
		ARCH=armel
		LZMA_LEVEL=9
	;;
	obsa7)
		ISOFILE=debian-7.1.0-armel-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=3
		ARCH=armel
		LZMA_LEVEL=9
		COMPRESS=gzip
		COMPRESS_EXT=gz
	;;
	obs600)
		KERNEL=2.6.32
		ISOFILE=debian-7.1.0-powerpc-DVD-1.iso
		RAMDISK_SIZE=144
		PATCHLEVEL=0
		ARCH=powerpc
		LZMA_LEVEL=9
		COMPRESS=gzip
		COMPRESS_EXT=gz
	;;
	*) exit 1 ;;
	esac
;;
squeeze)
#	ISOFILE=debian-6.0.5-armel-DVD-1.iso
	ARCH=armel
	case ${TARGET} in
	obsax3)
		RAMDISK_SIZE=128
		KERNEL=3.0.6
		# 2013/06/17
		PATCHLEVEL=14
	;;
	obsa6)
		RAMDISK_SIZE=128
		KERNEL=2.6.31
		# 2013/01/31
		PATCHLEVEL=8
		LZMA_LEVEL=9
	;;
	*) exit 1 ;;
	esac
;;
esac




if [ -f _config.sh ] ; then
	. _config.sh
elif [ -f ../_config.sh ] ; then
	. ../_config.sh
else
	echo "could't read _config.sh"
	exit 1
fi

