#!/bin/bash
#
# Copyright (c) 2013-2018 Plat'Home CO., LTD.
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

(cd ${DISTDIR}/dev/; rm -f rtc rtc0;  mknod rtc0 c 254 0) || exit

(cd ${DISTDIR}/dev/; rm -f sda*; mknod sda b 8 0)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sda${n} b 8 $((0+${n}));done)

(cd ${DISTDIR}/dev/; rm -f sdb*; mknod sdb b 8 16)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sdb${n} b 8 $((16+${n}));done)

(cd ${DISTDIR}/dev/; rm -f sdc*; mknod sdc b 8 32)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sdc${n} b 8 $((32+${n}));done)

(cd ${DISTDIR}/dev/; rm -f sdd*; mknod sdd b 8 48)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sdd${n} b 8 $((48+${n}));done)

(cd ${DISTDIR}/dev/; rm -f mtd0;  mknod mtd0 c 90 0)
(cd ${DISTDIR}/dev/; rm -f mtd1;  mknod mtd1 c 90 2)
(cd ${DISTDIR}/dev/; rm -f mtd2;  mknod mtd2 c 90 4)
(cd ${DISTDIR}/dev/; rm -f mtd3;  mknod mtd3 c 90 6)
(cd ${DISTDIR}/dev/; rm -f mtd4;  mknod mtd4 c 90 8)
(cd ${DISTDIR}/dev/; rm -f mtd5;  mknod mtd5 c 90 10)
(cd ${DISTDIR}/dev/; rm -f mtd6;  mknod mtd6 c 90 12)
