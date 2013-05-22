#!/bin/bash 

. `dirname $0`/../config.sh

TARGET=${1:-obsax3}
if [ "$TARGET" == "obsax3" ] ; then
	MODEL="-DCONFIG_OBSAX3"
#	LINUX_INC=/usr/src/ax3/linux-${KERNEL}/include
else
	MODEL="-DCONFIG_OBSA6"
#	LINUX_INC=/usr/src/a6/linux-${KERNEL}/include
fi

LINUX_INC=$(dirname $0)/../../source/${TARGET}/linux-${KERNEL}/include

CFLAGS="-Wall -I$LINUX_INC -DDEBIAN ${MODEL}"

if [ "$ARCH" == "armhf" ] ; then
#	CFLAGS="$CFLAGS -march=armv7-a -mhard-float -mfloat-abi=softfp -mfpu=vfpv3-d16"
	CFLAGS="$CFLAGS -DCONFIG_LINUX_3_2_X"
fi


echo "FLASHCFG"
$CC -lz -o flashcfg-debian flashcfg.c -DFLASHCFG_S -DEXTRACT_LZMA $CFLAGS

echo "RUNLED"
$CC -o runled runled.c $CFLAGS

echo "PSHD"
$CC -DHAVE_PUSHSW_OBSAXX_H -o pshd pshd.c $CFLAGS

echo;echo;echo
ls -l flashcfg-debian
ls -l runled
ls -l pshd

