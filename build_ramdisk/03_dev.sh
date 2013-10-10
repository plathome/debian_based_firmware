#!/bin/bash

. `dirname $0`/config.sh

if [ "$TARGET" == "obs600" && "$KERNEL" == "2.6.29" ]; then
(cd ${DISTDIR}/dev/; rm -f rtc rtc0;  mknod rtc0 c 10 135) || exit
else
(cd ${DISTDIR}/dev/; rm -f rtc rtc0;  mknod rtc0 c 254 0) || exit
fi

(cd ${DISTDIR}/dev/; rm -f mtd0; mknod mtd0 c 90 0)
(cd ${DISTDIR}/dev/; rm -f mtd1; mknod mtd1 c 90 2)
(cd ${DISTDIR}/dev/; rm -f mtd2; mknod mtd2 c 90 4)
(cd ${DISTDIR}/dev/; rm -f mtd3; mknod mtd3 c 90 6)
(cd ${DISTDIR}/dev/; rm -f mtd4; mknod mtd4 c 90 8)
(cd ${DISTDIR}/dev/; rm -f mtd5; mknod mtd5 c 90 10)
(cd ${DISTDIR}/dev/; rm -f mtd6; mknod mtd6 c 90 12)

(cd ${DISTDIR}/dev/; rm -f mtdblock0; mknod mtdblock0 b 31 0)
(cd ${DISTDIR}/dev/; rm -f mtdblock1; mknod mtdblock1 b 31 1)
(cd ${DISTDIR}/dev/; rm -f mtdblock2; mknod mtdblock2 b 31 2)
(cd ${DISTDIR}/dev/; rm -f mtdblock3; mknod mtdblock3 b 31 3)
(cd ${DISTDIR}/dev/; rm -f mtdblock4; mknod mtdblock4 b 31 4)
(cd ${DISTDIR}/dev/; rm -f mtdblock5; mknod mtdblock5 b 31 5)
(cd ${DISTDIR}/dev/; rm -f mtdblock6; mknod mtdblock6 b 31 6)

(cd ${DISTDIR}/dev/; rm -f sda*; mknod sda b 8 0)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sda${n} b 8 $((0+${n}));done)

(cd ${DISTDIR}/dev/; rm -f sdb*; mknod sdb b 8 16)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sdb${n} b 8 $((16+${n}));done)

(cd ${DISTDIR}/dev/; rm -f sdc*; mknod sdc b 8 32)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sdc${n} b 8 $((32+${n}));done)

(cd ${DISTDIR}/dev/; rm -f sdd*; mknod sdd b 8 48)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod sdd${n} b 8 $((48+${n}));done)

(cd ${DISTDIR}/dev/; rm -f uba*; mknod uba b 180 0)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod uba${n} b 180 $((0+${n}));done)

(cd ${DISTDIR}/dev/; rm -f ubb*; mknod ubb b 180 8)
(cd ${DISTDIR}/dev/; for n in 1 2;do mknod ubb${n} b 180 $((8+${n}));done)

