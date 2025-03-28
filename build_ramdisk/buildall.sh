#!/bin/bash
#
# Copyright (c) 2013-2025 Plat'Home CO., LTD.
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

#debug=echo
. `dirname $0`/config.sh

DIST_LIST="(stretch|buster|bullseye|bookworm|trixie)"
TARGET_LIST="(obshx1|obshx1r|obshx2|obshx2r|obsix9|obsix9r|obsa16|obsa16r|obsfx0|obsfx0r|obsfx1|obsfx1r|obsgx4|obsgx4r|obsduo|obstb3n|obsvx1|obsvx2|obsbx1|obsbx1s|obsa7|obsax3)"

function _usage(){
	echo
	echo "usage: $(basename $0) -M [obshx1|obshx1r|obshx2|obshx2r|obsix9|obsix9r|obsa16|obsa16r|obsfx0|obsfx0r|obsfx1|obsfx1r|obsgx4|obsgx4r|obsduo|obsvx1|obstb3n|obsvx2|obsbx1|obsbx1s|obsa7|obsax3] -D [stretch|buster|bullseye|bookworm|trixie]"
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

case $_DIST in
stretch)
	case $_TARGET in
	obsa16*|obsfx*|obsgx4*|obsduo|obstb3n|obsix9*|obshx*)
		echo
		echo "$_TARGET is not supported."
		exit 1
		;;
	esac
	;;
buster)
	case $_TARGET in
	obsa16*|obsfx*|obsgx4*|obsduo|obstb3n|obshx*)
		echo
		echo "$_TARGET is never supported."
		exit 1
		;;
	esac
	;;
bullseye)
	case $_TARGET in
	obsgx4*|obsduo)
		echo
		echo "$_TARGET is never supported."
		exit 1
		;;
	esac
	;;
bookworm)
	case $_TARGET in
	obsbx*|obshx2*|obstb3n)
		echo
		echo "$_TARGET is never supported."
		exit 1
		;;
	esac
	;;
trixie)
	case $_TARGET in
	obsbx*|obshx2*|obstb3n|obsduo)
		echo
		echo "$_TARGET is never supported."
		exit 1
		;;
	esac
	;;
*)
	;;
esac

SCRIPTS="build_debootstrap.sh build_kernel.sh build_ramdisk.sh release_firmware.sh"

for sh in $SCRIPTS; do
	DIST=${_DIST} TARGET=${_TARGET} ./$sh || exit 1
done
