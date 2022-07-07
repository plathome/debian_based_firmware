#!/bin/bash
#
# Copyright (c) 2013-2022 Plat'Home CO., LTD.
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

#TEST=echo

[ -f /etc/os-release ] && . /etc/os-release
[ -f /etc/default/openblocks ] && . /etc/default/openblocks

WORK_DIR=/tmp/_tmpfs.$$

# only openblocks IoT
function _get_md5() {
	MD5_RET=""

	if [ -f "${FIRM_FILE}/MD5.${MODEL}" ]; then
		IFS=$'\n'
		_file=(`cat ${FIRM_FILE}/MD5.${MODEL}`)
		IFS=$' '
		for line in "${_file[@]}"; do
			arry=(`echo $line`)
			if [ ${arry[1]} == "$1" ]; then
				MD5_RET=${arry[0]}
				break
			fi
		done
		if [ "$MD5_RET" == "" ]; then
			echo "$LINENO: MD5 file is broken, write firmware failed."
			exit 1
		fi
	else
		IFS=$' '
		MD5_RET=(`md5sum ${FIRM_FILE}/$1`)
	fi
}

function _usage() {
	echo
	echo "usage: $(basename $0) [-f directory] [-u list] [-bBeEpsSTly]"
	echo
	echo "    -f      directory Save firmware directory to FlashROM."
	echo "    -h      This messages."
	echo
	exit 1
}

function _yesno() {
	if [ "$FLG_YES" != "true" ] ; then
		msg=$1
		while true;do
			echo
			#echo "FlashROM overwrites the current data."
			echo "$msg"
			echo -n "Are you ok? [y|N] "
			read yesno
			case $yesno in
			[yY]|[yY][eE][sS])
				echo
				break
			;;
			*)
				exit 1
			;;
			esac
		done
	fi
}

# check mode, ramdisk or storage
function _check_mode() {
	cat /proc/mounts | grep -q "\/\.rw tmpfs"
	return $?
}



RUN=help

GETOPTS_ARG="f:hy"

while getopts $GETOPTS_ARG OPT;do
	case $OPT in
	f) RUN=firmware; FIRM_FILE=$OPTARG ;;
	y) FLG_YES=true;;
	h|*) _usage ;;
	esac
done

case $RUN in
firmware)
	case $MODEL in
	obsvx2|obsbx*)
		# check MD5
		for file in bzImage modules.tgz obstools.tgz
		do
			if [ -f ${FIRM_FILE}/${file} ]; then
				_get_md5 ${file}
				val=(`md5sum ${FIRM_FILE}/${file}`)
				if [ "$MD5_RET" != $val ]; then
					echo "$LINENO: ${file} is broken, write firmware failed."
					[ "$DEBUG" == "yes" ] && echo "${FIRM_FILE}/${file}: MD5.${MODEL}=$MD5_RET, source=$val"
					exit 1
				fi
			fi
		done

		# remove previous firmware
		mkdir -p ${WORK_DIR}
		if [ "$MODEL" == "obsvx2" ]; then
			mount `findfs LABEL=${FIRM_DIR}` ${WORK_DIR}
		else
			mount ${FIRM_DIR} ${WORK_DIR}
		fi

		# copy new firmware
		if [ -f ${FIRM_FILE}/bzImage ]; then
			${TEST} cp -f ${FIRM_FILE}/bzImage ${WORK_DIR}
		fi

		if [ -f ${FIRM_FILE}/openblocks-release ]; then
			${TEST} cp -f ${FIRM_FILE}/openblocks-release ${WORK_DIR}
		fi

		if [ -f ${FIRM_FILE}/obstools.tgz ]; then
			${TEST} tar xfzp ${FIRM_FILE}/obstools.tgz -C /
		fi

		if [ -f ${FIRM_FILE}/modules.tgz ]; then
			CPATH="/lib"; [ "$MODEL" == "obsbx1" ] && CPATH="/"
			${TEST} tar xfzp ${FIRM_FILE}/modules.tgz -C ${CPATH}
		fi

		if [ -f ${FIRM_FILE}/System.map ]; then
			${TEST} cp -f ${FIRM_FILE}/System.map /boot
		fi
		sync

		umount ${WORK_DIR}
		rmdir ${WORK_DIR}
		;;
	obsa16|obsfx1)
		# check MD5
		dtbfile=`find $FIRM_FILE -name "*\.dtb"`
		for file in Image $dtbfile modules.tgz
		do
			if [ -f ${FIRM_FILE}/${file} ]; then
				_get_md5 ${file}
				val=(`md5sum ${FIRM_FILE}/${file}`)
				if [ "$MD5_RET" != $val ]; then
					echo "$LINENO: ${file} is broken, write firmware failed."
					[ "$DEBUG" == "yes" ] && echo "${FIRM_FILE}/${file}: MD5.${MODEL}=$MD5_RET, source=$val"
					exit 1
				fi
			fi
		done

		# remove previous firmware
		mkdir -p ${WORK_DIR}
		mount ${FIRM_DIR} ${WORK_DIR}

		# copy new firmware
		if [ -f ${FIRM_FILE}/Image ]; then
			${TEST} cp -f ${FIRM_FILE}/Image ${WORK_DIR}
		fi

		${TEST} cp -f ${FIRM_FILE}/*.dtb ${WORK_DIR}

		if [ -f ${FIRM_FILE}/openblocks-release ]; then
			${TEST} cp -f ${FIRM_FILE}/openblocks-release ${WORK_DIR}
		fi

		if [ -f ${FIRM_FILE}/modules.tgz ]; then
			${TEST} tar xfzp ${FIRM_FILE}/modules.tgz -C /lib
		fi

		if [ -f ${FIRM_FILE}/System.map ]; then
			${TEST} cp -f ${FIRM_FILE}/System.map /boot
		fi
		sync

		umount ${WORK_DIR}
		rmdir ${WORK_DIR}
		;;
	*)
		;;
	esac
;;
*)
	_usage
;;
esac

exit 0
