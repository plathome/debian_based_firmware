#!/bin/bash

. `dirname $0`/config.sh

(cd ${SQUEEZEDIR}/etc;ln -sf ../usr/share/zoneinfo/Asia/Tokyo localtime)

