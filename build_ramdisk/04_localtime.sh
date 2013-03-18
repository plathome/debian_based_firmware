#!/bin/bash

. `dirname $0`/config.sh

(cd ${DISTDIR}/etc;ln -sf ../usr/share/zoneinfo/Asia/Tokyo localtime)

