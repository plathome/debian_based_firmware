
WRKDIR=$(cd $(dirname $0)/..; pwd)

DISTDIR=${WRKDIR}/${DIST}_${TARGET}

FILESDIR=${PWD}/files

ISOFILEDIR=${WRKDIR}/isofiles
EXTRADEBDIR=${WRKDIR}/extradebs/${DIST}

ETCDIR=${PWD}/etc.${DIST}
ETCDIR_ADD=${PWD}/etc.${DIST}.${TARGET}

RAMDISK_IMG=ramdisk-${DIST}.${TARGET}.img

if [ -z "$ARCH" ] ; then
	case $(uname -m) in
	arm*) ARCH=armel ;;
	ppc)  ARCH=powerpc ;;
	*)    ARCH=unknown ;;
	esac
fi

trap 'echo;echo "arch: ${ARCH}, dist: ${DIST}, target: ${TARGET}"' EXIT
