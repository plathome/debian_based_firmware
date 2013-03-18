#!/bin/bash 

TARGET=${1:-obsax3}
if [ "$TARGET" == "obsax3" ] ; then
	MODEL="-DCONFIG_OBSAX3"
	LINUX_INC=/usr/src/ax3/linux-3.0.6/include
else
	MODEL="-DCONFIG_OBSA6"
	LINUX_INC=/usr/src/a6/linux-2.6.31.8/include
fi

CFLAGS="-Wall -I$LINUX_INC -DDEBIAN ${MODEL}"

set -x

echo "FLASHCFG"
gcc -lz -o flashcfg-debian flashcfg.c -DFLASHCFG_S -DEXTRACT_LZMA $CFLAGS

echo "RUNLED"
gcc -o runled runled.c $CFLAGS

echo "PSHD"
gcc -DHAVE_PUSHSW_OBSAXX_H -o pshd pshd.c $CFLAGS

set +x
