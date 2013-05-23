#!/bin/bash

. `dirname $0`/config.sh

BUILDDIR=/tmp/obstools.$$

if [ "$TARGET" == "obsax3" ] ; then
	MODEL="-DCONFIG_OBSAX3"
#	LINUX_INC=/usr/src/ax3/linux-${KERNEL}/include
else
	MODEL="-DCONFIG_OBSA6"
#	LINUX_INC=/usr/src/a6/linux-${KERNEL}/include
fi

LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

CFLAGS="-Wall -I$LINUX_INC -DDEBIAN ${MODEL}"

if [ "$ARCH" == "armhf" ] ; then
#	CFLAGS="$CFLAGS -march=armv7-a -mhard-float -mfloat-abi=softfp -mfpu=vfpv3-d16"
	CFLAGS="$CFLAGS -DCONFIG_LINUX_3_2_X"
fi

mkdir -p ${BUILDDIR}

echo "FLASHCFG"
$CC -lz -o ${BUILDDIR}/flashcfg-debian ${FILESDIR}/flashcfg.c -DFLASHCFG_S -DEXTRACT_LZMA $CFLAGS

echo "RUNLED"
$CC -o ${BUILDDIR}/runled ${FILESDIR}/runled.c $CFLAGS

echo "PSHD"
$CC -DHAVE_PUSHSW_OBSAXX_H -o ${BUILDDIR}/pshd ${FILESDIR}/pshd.c $CFLAGS

echo;echo;echo
(cd ${BUILDDIR}; ls -l flashcfg-debian runled pshd)

cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
chmod 555 ${DISTDIR}/usr/sbin/flashcfg

cp ${FILESDIR}/usbreset.sh ${DISTDIR}/usr/sbin/usbreset
chmod 555 ${DISTDIR}/usr/sbin/usbreset

for cmd in flashcfg-debian runled pshd; do
	(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
	$STRIP ${DISTDIR}/usr/sbin/$cmd
done

rm -rf ${BUILDDIR}
