#!/bin/bash
#
# Copyright (c) 2013, 2014 Plat'Home CO., LTD.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY PLAT'HOME CO., LTD. AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL PLAT'HOME CO., LTD. AND CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

. `dirname $0`/config.sh

BUILDDIR=/tmp/obstools.$$

FLASHCFG="flashcfg.c"
if [ "$TARGET" == "obsax3" ] ; then
	MODEL="-DCONFIG_OBSAX3"
elif [ "$TARGET" == "obsa7" ] ; then
	MODEL="-DCONFIG_OBSA6 -DCONFIG_OBSA7"
elif [ "$TARGET" == "obsa6" ] ; then
	MODEL="-DCONFIG_OBSA6"
else
	FLASHCFG="flashcfg_obs600.c"
fi

LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

if [ "$KERNEL" == "3.13" -o "$KERNEL" == "3.10.25" ] ; then
CFLAGS="-Wall -I/usr/${KERN_ARCH}-linux-gnu${ABI}/include -L/usr/${KERN_ARCH}-linux-gnu${ABI}/lib -DDEBIAN ${MODEL}"
else
CFLAGS="-Wall -I$LINUX_INC -DDEBIAN ${MODEL}"
fi

if [ "$TARGET" == "obs600" ]; then
	CFLAGS+=" -DHAVE_PUSHSW_OBS600_H"
else
	CFLAGS+=" -DHAVE_PUSHSW_OBSAXX_H"
fi

if [ "$TARGET" == "obsax3" -a "$DIST" == "wheezy" ] ; then
	CFLAGS+=" -DCONFIG_LINUX_3_2_X"
fi

if [ "$KERNEL" == "3.13" ] ; then
	CFLAGS+=" -DCONFIG_LINUX_3_11_X"
fi

mkdir -p ${BUILDDIR}

echo "FLASHCFG"
$CC -lz -o ${BUILDDIR}/flashcfg-debian ${FILESDIR}/${FLASHCFG} -DFLASHCFG_S -DEXTRACT_LZMA $CFLAGS

echo "RUNLED"
$CC -o ${BUILDDIR}/runled ${FILESDIR}/runled.c $CFLAGS

echo "PSHD"
$CC -o ${BUILDDIR}/pshd ${FILESDIR}/pshd.c $CFLAGS

echo;echo;echo
(cd ${BUILDDIR}; ls -l flashcfg-debian runled pshd)

cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
chmod 555 ${DISTDIR}/usr/sbin/flashcfg

if [ "$TARGET" == "obsa6" -o "$TARGET" == "obsax3" ]; then
	cp ${FILESDIR}/usbreset.sh ${DISTDIR}/usr/sbin/usbreset
	chmod 555 ${DISTDIR}/usr/sbin/usbreset
fi

for cmd in flashcfg-debian runled pshd; do
	(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
	$STRIP ${DISTDIR}/usr/sbin/$cmd
done

rm -rf ${BUILDDIR}
