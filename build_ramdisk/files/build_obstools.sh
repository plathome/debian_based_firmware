#!/bin/bash 

. `dirname $0`/../config.sh

TARGET=${1:-obsax3}
if [ "$TARGET" == "obsax3" ] ; then
	MODEL="-DCONFIG_OBSAX3"
	LINUX_INC=/usr/src/ax3/linux-${KERNEL}/include
else
	MODEL="-DCONFIG_OBSA6"
	LINUX_INC=/usr/src/a6/linux-${KERNEL}/include
fi

CFLAGS="-Wall -I$LINUX_INC -DDEBIAN ${MODEL}"

if [ "$ARCH" == "armhf" ] ; then
	CFLAGS="$CFLAGS -march=armv7-a -mhard-float -mfloat-abi=softfp -mfpu=vfpv3-d16"
	CFLAGS="$CFLAGS -DCONFIG_LINUX_3_2_X"
fi


echo "FLASHCFG"
gcc -lz -o flashcfg-debian flashcfg.c -DFLASHCFG_S -DEXTRACT_LZMA $CFLAGS
ls -l flashcfg-debian

echo "RUNLED"
gcc -o runled runled.c $CFLAGS
ls -l runled

echo "PSHD"
gcc -DHAVE_PUSHSW_OBSAXX_H -o pshd pshd.c $CFLAGS
ls -l pshd

