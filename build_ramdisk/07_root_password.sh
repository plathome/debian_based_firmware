#!/bin/bash

. `dirname $0`/config.sh

pw='$1$afC9J.v6$Dkq.k8sVRq7n0Py5eWWAp1'

chroot ${DISTDIR} /usr/sbin/usermod -p $pw root
