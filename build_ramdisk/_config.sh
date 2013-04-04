
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

trap 'echo;echo "arch: ${ARCH}, dist: ${DIST}, target: ${TARGET}"' EXIT
