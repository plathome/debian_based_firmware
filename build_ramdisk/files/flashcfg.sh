#!/bin/bash
#
# Copyright (c) 2013-2016 Plat'Home CO., LTD.
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

[ -f /etc/default/openblocks ] && . /etc/default/openblocks

RW_DIR="${RW_DIR:=/.rw}"
RW_CF_LABEL="${RW_CF_LABEL:=DEBIAN}"

WORK_DIR=/tmp/_tmpfs.$$
SAVE_FILE=userland.tgz
SAVE_FILE_SIZE=0

MTD_RO=0x800
MTD_RW=0xC00

MTREE_LIST="${MTREE_LIST:=/etc/mtree.list}"
MTREE_DIRS="${MTREE_DIRS:=/var/log /var/run}"

SAVE_FILE_REC=/etc/flashcfg.save
[ -f "${SAVE_FILE_REC}" ] && . ${SAVE_FILE_REC}
USED_CONF="${USED_CONF:=unknown}"
USED_USER="${USED_USER:=unknown}"

_MTD_CONF_SIZE=$(mawk "/${MTD_CONF_DEV/block/}/ {print \$2}" /proc/mtd 2> /dev/null)
MTD_CONF_SIZE=$(($(printf "%d" 0x${_MTD_CONF_SIZE:-0})/1024))
MTD_CONF_SIZE_MB=$((${MTD_CONF_SIZE}/1024))
_MTD_USER_SIZE=$(mawk "/${MTD_USER_DEV/block/}/ {print \$2}" /proc/mtd 2> /dev/null)
MTD_USER_SIZE=$(($(printf "%d" 0x${_MTD_USER_SIZE:-0})/1024))
MTD_USER_SIZE_MB=$((${MTD_USER_SIZE}/1024))

#if ! which flashcp > /dev/null 1>&1 ; then
#	echo
#	echo "This program needs to \"mtd-utils\" package."
#	echo "Plaese install \"mtd-utils\" packages."
#	echo
#	echo "ex) aptitude update && aptitude install mtd-utils"
#	echo
#	exit 1
#fi

trap "_all_protect_mtd; _umount_wrkdir" EXIT TERM QUIT

function _all_protect_mtd() {
	for mtd in $MTD_FIRM_DEV $MTD_CONF_DEV $MTD_USER_DEV $MTD_OPT_DEV;do
		[ -f /sys/devices/virtual/mtd/${mtd}/flags ] && \
		echo $MTD_RO > /sys/devices/virtual/mtd/${mtd}/flags
	done
}

function _protect_mtd() {
	local mtd=$1
	local mode=$2 # 1 = on, 0 = off
	local value=
	[ -z "$mtd" ] && return 1
	[ ! -f "/sys/devices/virtual/mtd/${mtd}/flags" ] && return 1
	if [ "$mode" == "1" ] ; then
		value=$MTD_RO
	else
		value=$MTD_RW
	fi
	echo "$value" > /sys/devices/virtual/mtd/${mtd}/flags
}

function _umount_wrkdir() {
	if mount -n 2> /dev/null | grep -q "${WORK_DIR}" ; then
		umount ${WORK_DIR}
	fi
	rm -rf ${WORK_DIR}
}

function _usage() {
	echo
	echo "Archiving userland files to FlashROM and some configuration."
	echo
	#echo "usage: $(basename $0) [-c type] [-f file] [-u list] [-bBeEsSly]"
	echo "usage: $(basename $0) [-f file] [-u list] [-bBeEpsSTly]"
	echo
	echo "    -b      Save config to BACKUP(LABEL=DEB_CONFIG) storage."
	echo "    -B      Save userland and config to BACKUP(LABEL=DEB_CONFIG) storage."
	echo "    -u list Save files from list to BACKUP(LABEL=DEB_CONFIG) storage."
	#echo "    -c type Change boot setting [initrd|cf|sda[1-8]]."
	echo "    -f file Save firmware file to FlashROM."
	echo "    -s      Save config to FlashROM (${MTD_CONF_DEV})."
	echo "    -S      Save userland and config to FlashROM (${MTD_USER_DEV})."
	echo "            '-s' will run concurrently."
	echo "    -e      Erase FlashROM (header only)."
	echo "    -E      Erase FlashROM (all clear)."
	echo "    -l      Show last save size."
	echo "    -y      Assume yes for save and erase."
#	echo "    -t      Test read and write in coredump save area."
#	echo "    -T      Read messages in coredump save area."
if [ "$MODEL" == "obsax3" ] ; then
	echo "    -p [pm] Print or set power management level. [now|wfi|idle|snooze]"
fi
	echo "    -h      This messages."
	echo
	exit 1
}

function _save_size_rec() {
	cat <<-_EOB > ${SAVE_FILE_REC}
USED_CONF=${USED_CONF}
USED_USER=${USED_USER}
	_EOB
}

function _save_size_clear() {
	cat <<-_EOB > ${SAVE_FILE_REC}
USED_CONF=0
USED_USER=0
	_EOB
}

function _save_mtree_list() {
	for d in ${MTREE_DIRS}; do
		find ${d} -type d -ls|mawk '{print $5":"$6":"$11}'
	done > ${MTREE_LIST}
}


function _save_userland() {
	_save_mtree_list
	echo -n "Archiving userland files... "
	if (cd ${RW_DIR};tar -X /etc/exclude.list -cpzf ${WORK_DIR}/${SAVE_FILE} .); then
		SAVE_FILE_SIZE=$(ls -s ${WORK_DIR}/${SAVE_FILE} | mawk '{print $1}')
		echo "done (Approximately $((SAVE_FILE_SIZE/1024)) MBytes)"
		USED_USER=${SAVE_FILE_SIZE}
		_save_size_rec
		return 0
	else
		echo "fail"
		return 1
	fi
}

function _save_config() {
	_save_mtree_list
	echo -n "Archiving /etc config files... "
	if (cd ${RW_DIR};tar -X /etc/exclude.list -cpzf ${WORK_DIR}/${SAVE_FILE} etc/); then
		SAVE_FILE_SIZE=$(ls -s ${WORK_DIR}/${SAVE_FILE} | mawk '{print $1}')
		echo "done (Approximately $((SAVE_FILE_SIZE)) KBytes)"
		USED_CONF=${SAVE_FILE_SIZE}
		_save_size_rec
		return 0
	else
		echo "fail"
		return 1
	fi
}

function _del_flashrom() {
	echo ${MTD_CONF_DEV} | grep -q 'mtd' || MODEL=_obsa6_but_dev_is_non-mtd
	echo -n "Eraing FlashROM... "
	if [ "$MODEL" == "obsa6" -o "$MODEL" == "obsa7" ] ; then
		flashcfg-debian -d 2> /dev/null
		flashcfg-debian -D 2> /dev/null
	else
		for dev in ${MTD_CONF_DEV} ${MTD_USER_DEV} ;do
			_protect_mtd $dev 0
			dd if=/dev/zero of=/dev/$dev count=1 > /dev/null 2>&1
			_protect_mtd $dev 1
		done
	fi
	echo "done"
}

function _del_all_flashrom() {
	echo ${MTD_CONF_DEV} | grep -q 'mtd' || MODEL=_obsa6_but_dev_is_non-mtd
	echo -n "Eraing FlashROM... "
	if [ "$MODEL" == "obsa6" -o "$MODEL" == "obsa7" ] ; then
		flashcfg-debian -d 2> /dev/null
		flashcfg-debian -D 2> /dev/null
	else
		for dev in ${MTD_CONF_DEV} ${MTD_USER_DEV} ;do
			_protect_mtd $dev 0
			dd if=/dev/zero of=/dev/$dev > /dev/null 2>&1
			_protect_mtd $dev 1
		done
	fi
	echo "done"
}

function _backup_files() {
	if mount LABEL=DEB_CONFIG /mnt 2> /dev/null ; then
		dir=${RW_DIR}
		if [ "${FLG_ALLBKUP}" == "true" ] ; then
			echo -n "Archiving userland files... "
			TAR_ARG="-cpzf /mnt/${SAVE_FILE} ."
		elif [ "${FLG_LISTBKUP}" == "true" ] ; then
			echo -n "Archiving userland files from list file... "
			TAR_ARG="-cpzf /mnt/${SAVE_FILE} -T ${FILE_LIST}"
			dir=/
		else
			echo -n "Archiving /etc config files... "
			TAR_ARG="-cpzf /mnt/${SAVE_FILE} etc/"
		fi
		grep -v 'usr/lib/jre' /etc/exclude.list > /tmp/.exclude.list
		if (cd $dir;tar -X /tmp/.exclude.list ${TAR_ARG} 2> /dev/null); then
			echo "done"
			ret=0
		else
			echo "fail"
			ret=1
		fi
		rm -f /tmp/.exclude.list
		umount /mnt
		return $ret
	else
		echo
		echo "No such a LABEL=DEB_CONFIG storage."
		echo
		return 1
	fi
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



RUN=help

if [ "$MODEL" == "obsax3" -o "$MODEL" == "obs600" ] ; then
	GETOPTS_ARG="c:f:p:u:bBeEsSlhxXyZ"
else
	GETOPTS_ARG="c:f:u:x::X::bBeEsSlhyZ"
fi

while getopts $GETOPTS_ARG OPT;do
	case $OPT in
	c) RUN=rootcfg; ROOT_TARGET=$OPTARG ;;
	f) RUN=firmware; FIRM_FILE=$OPTARG; MTD_DEV=/dev/${MTD_CONF_DEV} ;;
	b) RUN=backup ;;
	B) RUN=backup; FLG_ALLBKUP=true ;;
	u) RUN=backup; FLG_LISTBKUP=true; FILE_LIST=$OPTARG ;;
	e) RUN=delete ;;
	E) RUN=delete; FLG_ALLDEL=true;;
	Z) RUN=save_default ;;
	p) RUN=pm; pm_level=$OPTARG ;;
	s)
		RUN=save
		ROM_SIZE=${MTD_CONF_SIZE}
		MTD_DEV=/dev/${MTD_CONF_DEV}
		COMMAND=_save_config
		FLASHCFG_ARG="-s"
	;;
	S)
		RUN=save
		FLG_ALLSAVE=true 
		ROM_SIZE=${MTD_USER_SIZE}
		MTD_DEV=/dev/${MTD_USER_DEV}
		COMMAND=_save_userland
		FLASHCFG_ARG="-S"
	;;
	t) RUN=coredump; CMDARG=t ;;
	T) RUN=coredump; CMDARG=T ;;
	x|X) RUN=extract ;;
	l) RUN=show ;;
	y) FLG_YES=true;;
	h|*) _usage ;;
	esac
done

case $RUN in
show)
	[ "${MTD_USER_SIZE_MB}" -eq 0 ] && MTD_USER_SIZE_MB=unknown
	[ "${MTD_CONF_SIZE_MB}" -eq 0 ] && MTD_CONF_SIZE_MB=unknown
	echo
	echo "Show FlashROM last saved size"
	echo
	echo -n " Use userland(-S)    : "
	if [ "${USED_USER}" == "unknown" ] ; then
		echo "${USED_USER} KBytes (MAX: ${MTD_USER_SIZE_MB} MBytes)"
	else
		echo "${USED_USER} KBytes (MAX: ${MTD_USER_SIZE_MB} MBytes)"
	fi
	echo " Use /etc config(-s) : ${USED_CONF} KBytes (MAX: ${MTD_CONF_SIZE_MB} MBytes)"
	echo
;;
save)
	_yesno "FlashROM overwrites the current data."

	mkdir -p ${WORK_DIR}
	[ "${ROM_SIZE}" -eq 0 ] && ROM_SIZE=$((512*1024))
	mount -t tmpfs -o size=${ROM_SIZE}k none ${WORK_DIR}
	
	if ${COMMAND}; then
		if echo ${MTD_DEV} | grep -q '/dev/mtd'; then
			flashcfg-debian ${FLASHCFG_ARG} ${WORK_DIR}/${SAVE_FILE} || exit 1
		else
			dd if=${WORK_DIR}/${SAVE_FILE} of=${MTD_DEV}
		fi
		echo
	else
		echo
		echo "ERROR: Can't archiving files. (${MTD_DEV})"
		echo
	fi

	# umount & rmdir
	_umount_wrkdir

	if [ "${FLG_ALLSAVE}" == "true" ] ; then
		$0 -y -s
	fi
;;
delete)
	if [ "$FLG_ALLDEL" == "true" ] ; then
		_yesno "Erase FlashROM (all userarea)."
		_del_all_flashrom
	else
		_yesno "Erase FlashROM (header only)."
		_del_flashrom
	fi
	_save_size_clear
;;
backup)
	_backup_files
;;
firmware)
	_yesno "Save firmware file to FlashROM."
	flashcfg-debian -f $FIRM_FILE
;;
rootcfg)
	case $ROOT_TARGET in
	initrd|cf|ext|sda*) flashcfg-debian -c $ROOT_TARGET ;;
	*) echo;echo "ERROR: unsupportted device"; echo; exit 1;;
	esac
;;
rootshow)
	flashcfg-debian -b
;;
extract)
	# for call from kernel init
	exit 0
;;
coredump)
	flashcfg-debian -${CMDARG}
;;
pm)
	flashcfg-debian -p ${pm_level:-now}
;;
save_default)
	_yesno "Clear FlashROM userare and save /etc/default/openblocks file."
	_del_flashrom
	target=$(awk '{gsub(":", "")} /config/ {print $1}' < /proc/mtd)
	echo "0xC00" > /sys/devices/virtual/mtd/${target}/flags
	(cd /; tar cvzf /tmp/.openblocks.tgz etc/default/openblocks)
	flashcfg-debian -s /tmp/.openblocks.tgz
	rm -f /tmp/.openblocks.tgz
	echo "0x800" > /sys/devices/virtual/mtd/${target}/flags
;;
*)
	_usage
;;
esac

exit 0
