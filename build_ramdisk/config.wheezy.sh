
export TARGET
TARGET=${TARGET:=obsax3}

DIST=wheezy

case ${TARGET} in
obsax3)
	RAMDISK_SIZE=128
	KERNEL=3.0.6
	#KERNEL=3.2.36
	# 2013/xx/xx
	PATCHLEVEL=0
;;
obsa6)
	RAMDISK_SIZE=128
	KERNEL=3.2.36
	# 2013/xx/xx
	PATCHLEVEL=0
;;
*) exit 1 ;;
esac

################################################################################

WRKDIR=$(cd $(dirname $0)/..; pwd)

DISTDIR=${WRKDIR}/${DIST}_${TARGET}

FILESDIR=${PWD}/files

ETCDIR=${PWD}/etc.${DIST}
ETCDIR_ADD=${PWD}/etc.${DIST}.${TARGET}

RAMDISK_IMG=ramdisk-${DIST}.${TARGET}.img

case $(uname -m) in
arm*) ARCH=armel ;;
ppc)  ARCH=powerpc ;;
*)    ARCH=unknown ;;
esac

