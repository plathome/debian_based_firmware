
DIST=wheezy

TARGET=${TARGET:=obsax3}

export TARGET DIST

case ${DIST} in
wheezy)
	case ${TARGET} in
	obsax3)
		RAMDISK_SIZE=160
		KERNEL=3.2.36
		# 2013/xx/xx
		PATCHLEVEL=0
	;;
	obsa6)
		RAMDISK_SIZE=128
		KERNEL=3.2.36
		PATCHLEVEL=0
	;;
	*) exit 1 ;;
	esac
;;
squeeze)
	case ${TARGET} in
	obsax3)
		RAMDISK_SIZE=128
		KERNEL=3.0.6
		# 2013/03/06
		PATCHLEVEL=11
	;;
	obsa6)
		RAMDISK_SIZE=128
		KERNEL=2.6.31
		# 2013/01/31
		PATCHLEVEL=8
	;;
	*) exit 1 ;;
	esac
;;
esac

. _config.sh

