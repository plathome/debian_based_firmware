
export TARGET
TARGET=${TARGET:=obsax3}

DIST=squeeze

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

################################################################################

WRKDIR=$(cd $(dirname $0)/..; pwd)

DISTDIR=${WRKDIR}/${DIST}_${TARGET}

FILESDIR=${PWD}/files
FILESDIR_ADD=${PWD}/files.${TARGET}

RAMDISK_IMG=ramdisk-${DIST}.${TARGET}.img

case $(uname -m) in
arm*) ARCH=armel ;;
ppc)  ARCH=powerpc ;;
*)    ARCH=unknown ;;
esac

