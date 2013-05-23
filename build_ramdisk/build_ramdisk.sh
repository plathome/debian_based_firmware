#!/bin/bash

. `dirname $0`/config.sh

#case $1 in
#*obsax3*) TARGET=obsax3 ;;
#*obsa6*)  TARGET=obsa6 ;;
#*)        exit 1 ;;
#esac

for sh in [0-9][0-9]_*.sh;do
	TARGET=$TARGET ./${sh} || exit 1
	sleep 1
done
