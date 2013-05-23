#!/bin/bash
#debug=echo
. `dirname $0`/config.sh

DIST_LIST="(squeeze|wheezy)"
TARGET_LIST="(obsax3|obsa6)"

function _usage(){
	echo
	echo "usage: $(basename $0) -M [obsax3|obsa6] -D [squeez|wheezy]"
	echo
	exit 1
}


[ "$#" -ne "4" ] && _usage

GETOPTS_ARG="D:M:"

while getopts $GETOPTS_ARG OPT;do
	case $OPT in
	D) _DIST=$OPTARG ;;
	M) _TARGET=$OPTARG ;;
	h|*) _usage ;;
	esac
done

if ! (echo $_DIST | grep -Eq "$DIST_LIST"); then
	echo "unknown dist"
	_usage
fi

if ! (echo $_TARGET | grep -Eq "$TARGET_LIST") ; then
	echo "unknown model."
	_usage
fi

SCRIPTS="build_debootstrap.sh build_ramdisk.sh build_kernel.sh release_firmware.sh"

for sh in $SCRIPTS; do
	DIST=${_DIST} TARGET=${_TARGET} ./$sh
done




