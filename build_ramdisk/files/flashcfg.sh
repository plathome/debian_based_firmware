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

[ -f /etc/os-release ] && . /etc/os-release
[ -f /etc/default/openblocks ] && . /etc/default/openblocks

if [ "$MODEL" == "obsvx2" ]; then
	echo "obsvx2 is not supported."
	exit
fi

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

# only openblocks bx1
function _get_md5() {
	MD5_RET=""

	if [ $1 == "kernel" ]; then
		obj="bzImage"
	elif [ $1 == "ramdisk" ]; then
		case $MODEL in
		obsbx1|obsvx1|obsix9|obsa16|obsfx1)
			if [ -e "${FIRM_FILE}/initrd.gz" ]; then
				obj="initrd.gz"
			else
				obj="ramdisk-wheezy.${MODEL}.img.gz"
			fi
			;;
		*)
			obj="ramdisk-wheezy.${MODEL}.img.gz"
			;;
		esac
	fi

	if [ -f "${FIRM_FILE}/MD5.${MODEL}" ]; then
		IFS=$'\n'
		file=(`cat ${FIRM_FILE}/MD5.${MODEL}`)
		IFS=$' '
		for line in "${file[@]}"; do
			arry=(`echo $line`)
			if [ ${arry[1]} == "$obj" ]; then
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
		MD5_RET=(`md5sum ${FIRM_FILE}/$obj`)
	fi
}

function _all_protect_mtd() {
	case $MODEL in
	obsa*)
	for mtd in $MTD_FIRM_DEV $MTD_CONF_DEV $MTD_USER_DEV $MTD_OPT_DEV;do
		[ -f /sys/devices/virtual/mtd/${mtd}/flags ] && \
		echo $MTD_RO > /sys/devices/virtual/mtd/${mtd}/flags
	done
	;;
	*);;
	esac
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
	case $MODEL in
	obsa*)
	if mount -n 2> /dev/null | grep -q "${WORK_DIR}" ; then
		umount ${WORK_DIR}
	fi
	rm -rf ${WORK_DIR}
	;;
	*);;
	esac
}

function _usage() {
	echo
	echo "Archiving userland files to eMMC and some configuration."
	echo
case $MODEL in
obsa*)
	echo "usage: $(basename $0) [-f file] [-u list] [-bBeEpsSTly]"
	;;
obsbx1|bpv*|obsvx1|obsix9|obsa16|obsfx1)
	echo "usage: $(basename $0) [-f directory] [-u list] [-bBeEpsSTly]"
	;;
*)
	;;
esac
	echo
	echo "    -b      Save config to BACKUP(LABEL=DEB_CONFIG) storage."
	echo "    -B      Save userland and config to BACKUP(LABEL=DEB_CONFIG) storage."
	echo "    -u list Save files from list to BACKUP(LABEL=DEB_CONFIG) storage."
	#echo "    -c type Change boot setting [initrd|cf|sda[1-8]]."
case $MODEL in
obsa*)
	echo "    -f file Save firmware file to FlashROM."
	echo "    -s      Save config to FlashROM (${MTD_CONF_DEV})."
	echo "    -S      Save userland and config to FlashROM (${MTD_USER_DEV})."
	echo "            '-s' will run concurrently."
	echo "    -e      Erase FlashROM (header only)."
	echo "    -E      Erase FlashROM (all clear)."
	;;
obsbx1|bpv*|obsvx1|obsix9|obsa16|obsfx1)
	echo "    -f directory Save firmware directory to eMMC."
	echo "    -s      Save config to eMMC (${MTD_CONF_DEV})."
	echo "    -S      Save userland and config to eMMC (${MTD_USER_DEV})."
	echo "            '-s' will run concurrently."
	echo "    -e      Erase eMMC (header only)."
	echo "    -E      Erase eMMC (all clear)."
	;;
*)
	;;
esac
	echo "    -l      Show last save size."
	echo "    -y      Assume yes for save and erase."
#	echo "    -t      Test read and write in coredump save area."
#	echo "    -T      Read messages in coredump save area."
if [ "$MODEL" == "obsax3" ] ; then
if [ "$VERSION_ID" == "8" ] ; then
	echo "    -p [pm] Print or set power management level. [now|dis|wfi|idle|snooze]"
else
	echo "    -p [pm] Print or set power management level. [now|wfi|idle|snooze]"
fi
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
		_save_mtree_list
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

# check mode, ramdisk or storage
function _check_mode() {
	cat /proc/mounts | grep -q "\/\.rw tmpfs"
	return $?
}



RUN=help

case $MODEL in
obsax3|obs600)
 	GETOPTS_ARG="c:f:p:u:bBeEsSlhxXyZ"
	;;
obsmv4)
	GETOPTS_ARG="f:u:bBlhy"
	;;
bpv4*|bpv8|obsvx1|obsix9|obsa16|obsfx1)
	GETOPTS_ARG="f:u:bBeEsSlhy"
	;;
obsbx1)
	GETOPTS_ARG="f:u:bBeEsSlhyoc"
	;;
*)
 	GETOPTS_ARG="c:f:u:x::X::bBeEsSlhyZ"
	;;
esac

while getopts $GETOPTS_ARG OPT;do
	case $OPT in
	c)
		case $MODEL in
		obsbx1|bpv*|obsvx1|obsix9|obsa16|obsfx1)
			RUN=rootcfg; ROOT_TARGET=$2
			;;
		*)
			RUN=rootcfg; ROOT_TARGET=$OPTARG
			;;
		esac
		;;
	f) RUN=firmware; FIRM_FILE=$OPTARG; MTD_DEV=/dev/${MTD_CONF_DEV} ;;
	b) RUN=backup ;;
	B) RUN=backup; FLG_ALLBKUP=true ;;
	u) RUN=backup; FLG_LISTBKUP=true; FILE_LIST=$OPTARG ;;
	e) RUN=delete ;;
	E) RUN=delete; FLG_ALLDEL=true;;
	Z) RUN=save_default ;;
	p) RUN=pm; pm_level=$OPTARG ;;
	s)
		if ! _check_mode; then
			echo "Storage mode does not save."
			exit 1
		fi
		case $MODEL in
		bpv4*|bpv8|obsbx1|obsvx1|obsix9|obsa16|obsfx1)
			RUN=save_direct_etc
			;;
		*)
			RUN=save
			ROM_SIZE=${MTD_CONF_SIZE}
			MTD_DEV=/dev/${MTD_CONF_DEV}
			COMMAND=_save_config
			FLASHCFG_ARG="-s"
			;;
		esac
		;;
	S)
		if ! _check_mode; then
			echo "Storage mode does not save."
			exit 1
		fi
		case $MODEL in
		bpv4*|bpv8|obsbx1|obsvx1|obsix9|obsa16|obsfx1)
			RUN=save_direct_user
			;;
		*)
			RUN=save
			FLG_ALLSAVE=true 
			ROM_SIZE=${MTD_USER_SIZE}
			MTD_DEV=/dev/${MTD_USER_DEV}
			COMMAND=_save_userland
			FLASHCFG_ARG="-S"
			;;
		esac
		;;
	t) RUN=coredump; CMDARG=t ;;
	T) RUN=coredump; CMDARG=T ;;
	x|X) RUN=extract ;;
	l) RUN=show ;;
	o) RUN=chgos; osname=$2 ;;
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
save_direct_etc)
	_yesno "Overwrites the current data."
	mkdir -p ${WORK_DIR}
	if [ "$MODEL" == "obsvx1" ]; then
		mount `findfs LABEL=${SAVE_DIR}` ${WORK_DIR}
	elif [ "$MODEL" == "obsix9" ]; then
		mount `findfs LABEL=${SAVE_DIR}` ${WORK_DIR}
	else
		mount ${SAVE_DIR} ${WORK_DIR}
	fi
	_save_mtree_list
	echo -n "Archiving /etc config files... "
	if (cd ${RW_DIR};tar -X /etc/exclude.list -cpzf ${WORK_DIR}/etc.tgz etc/); then
		SAVE_FILE_SIZE=$(ls -s ${WORK_DIR}/etc.tgz | mawk '{print $1}')
		echo "done (Approximately $SAVE_FILE_SIZE KBytes)"
		USED_CONF=${SAVE_FILE_SIZE}
		_save_size_rec
	else
		echo "fail"
	fi
	umount ${WORK_DIR}
	rm -rf ${WORK_DIR}
;;
save_direct_user)
	_yesno "Overwrites the current data."
	mkdir -p ${WORK_DIR}
	if [ "$MODEL" == "obsvx1" ]; then
		mount `findfs LABEL=${SAVE_DIR}` ${WORK_DIR}
	elif [ "$MODEL" == "obsix9" ]; then
		mount `findfs LABEL=${SAVE_DIR}` ${WORK_DIR}
	else
		mount ${SAVE_DIR} ${WORK_DIR}
	fi
	_save_mtree_list
	echo -n "Archiving userland files... "
	if (cd ${RW_DIR};tar -X /etc/exclude.list --exclude etc --exclude tmp -cpzf ${WORK_DIR}/userland.tgz .); then
		SAVE_FILE_SIZE=$(ls -s ${WORK_DIR}/userland.tgz | mawk '{print $1}')
		echo "done (Approximately $SAVE_FILE_SIZE KBytes)"
		USED_USER=${SAVE_FILE_SIZE}
		_save_size_rec
	else
		echo "fail"
	fi
	echo -n "Archiving /etc config files... "
	if (cd ${RW_DIR};tar -X /etc/exclude.list -cpzf ${WORK_DIR}/etc.tgz etc/); then
		SAVE_FILE_SIZE=$(ls -s ${WORK_DIR}/etc.tgz | mawk '{print $1}')
		echo "done (Approximately $SAVE_FILE_SIZE KBytes)"
		USED_CONF=${SAVE_FILE_SIZE}
		_save_size_rec
	else
		echo "fail"
	fi
	umount ${WORK_DIR}
	rm -rf ${WORK_DIR}
;;
save)
	_yesno "FlashROM(or eMMC) overwrites the current data."

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
	case $MODEL in
	bpv4*|bpv8|obsbx1|obsvx1|obsix9|obsa16|obsfx1)
		_yesno "Erase userarea)."

		mkdir -p ${WORK_DIR}
		if [ "$MODEL" == "obsvx1" ]; then
			mount `findfs LABEL=${SAVE_DIR}` ${WORK_DIR}
		elif [ "$MODEL" == "obsix9" ]; then
			mount `findfs LABEL=${SAVE_DIR}` ${WORK_DIR}
		else
			mount ${SAVE_DIR} ${WORK_DIR}
		fi
		rm -rf ${WORK_DIR}/etc.tgz
		rm -rf ${WORK_DIR}/userland.tgz
		umount ${WORK_DIR}
		;;
	*)
		if [ "$FLG_ALLDEL" == "true" ] ; then
			_yesno "Erase FlashROM (all userarea)."
			_del_all_flashrom
		else
			_yesno "Erase FlashROM (header only)."
			_del_flashrom
		fi
		_save_size_clear
		;;
	esac
	;;
backup)
	_backup_files
;;
firmware)
	case $MODEL in
	bpv4*|bpv8)
		mkdir -p ${WORK_DIR}
		if [ "$MODEL" == "obsvx1" -o "$MODEL" == "obsix9" ]; then
			mount `findfs LABEL=${FIRM_DIR}` ${WORK_DIR}
		else
			mount ${FIRM_DIR} ${WORK_DIR}
		fi
		cp -f ${FIRM_FILE}/bzImage ${WORK_DIR}/boot
		cp -f ${FIRM_FILE}/ramdisk-bpv.img.gz ${WORK_DIR}/boot
		if [ -f ${FIRM_FILE}/grub.cfg ]; then
			cp -f ${FIRM_FILE}/grub.cfg ${WORK_DIR}/boot/grub
		fi
		if [ -f ${FIRM_FILE}/openblocks-release ]; then
			cp -f ${FIRM_FILE}/openblocks-release ${WORK_DIR}
		fi
		umount ${WORK_DIR}
		rm -rf ${WORK_DIR}
	;;
	obsbx1|obsvx1|obsix9|obsa16|obsfx1)
		if [ -e "${FIRM_FILE}/initrd.gz" ]; then
			RAMDISK="initrd.gz"
#			_RAMDISK="ramdisk-wheezy.obsbx1.img.gz"
		else
			RAMDISK="ramdisk-wheezy.obsbx1.img.gz"
#			_RAMDISK="initrd.gz"
		fi
		# check MD5
		_get_md5 ramdisk
		val=(`md5sum ${FIRM_FILE}/${RAMDISK}`)
		if [ "$MD5_RET" != $val ]; then
			echo "$LINENO: ${RAMDISK} is broken, write firmware failed."
			if [ "$DEBUG" == "yes" ]; then
				echo "${FIRM_FILE}/${RAMDISK}: MD5.${MODEL}=$MD5_RET, source=$val"
			fi
			exit 1
		fi
		# check MD5
		_get_md5 kernel
		val=(`md5sum ${FIRM_FILE}/bzImage`)
		if [ "$MD5_RET" != $val ]; then
			echo "$LINENO: bzImage is broken, write firmware failed."
			if [ "$DEBUG" == "yes" ]; then
				echo "MD5.${MODEL}=$MD5_RET, source=$val"
			fi
			exit 1
		fi

		mkdir -p ${WORK_DIR}
		if [ "$MODEL" == "obsvx1" -o "$MODEL" == "obsix9" ]; then
			mount `findfs LABEL=${FIRM_DIR}` ${WORK_DIR}
		else
			mount ${FIRM_DIR} ${WORK_DIR}
		fi
		rm -f ${WORK_DIR}/openblocks-release
		rm -f ${WORK_DIR}/bzImage
		rm -f ${WORK_DIR}/${RAMDISK}
#		rm -f ${WORK_DIR}/${RAMDISK} ${WORK_DIR}/${_RAMDISK}

		# ramdisk
		_get_md5 ramdisk
		for i in {1..5}; do
			cp -f ${FIRM_FILE}/${RAMDISK} ${WORK_DIR}
			if [ $? != 0 ]; then
				echo "$LINENO: cp command error, goto retry"
			fi
			IFS=$' '
			val=(`md5sum ${WORK_DIR}/${RAMDISK}`)
			if [ "$MD5_RET" == $val ]; then
				break;
			fi
			if [ "$DEBUG" == "yes" ]; then
				echo "${WORK_DIR}/${RAMDISK}: MD5.${MODEL}=$MD5_RET, dest=$val"
			fi
		done
		if [ $i == 5 ]; then
			echo "$LINENO: retry over, write firmware failed."
			exit 1
		fi

		# kernel
		_get_md5 kernel
		for i in {1..5}; do
			cp -f ${FIRM_FILE}/bzImage ${WORK_DIR}
			if [ $? != 0 ]; then
				echo "$LINENO: cp command error, goto retry"
			fi
			IFS=$' '
			val=(`md5sum ${WORK_DIR}/bzImage`)
			if [ "$MD5_RET" == $val ]; then
				break;
			fi
			if [ "$DEBUG" == "yes" ]; then
				echo "MD5.${MODEL}=$MD5_RET, dest=$val"
			fi
		done
		if [ $i == 5 ]; then
			echo "$LINENO: retry over, write firmware failed."
			exit 1
		fi

		if [ -f ${FIRM_FILE}/openblocks-release ]; then
			cp -f ${FIRM_FILE}/openblocks-release ${WORK_DIR}
		fi
		umount ${WORK_DIR}
		rmdir ${WORK_DIR}
		;;
	*)
		_yesno "Save firmware file to FlashROM."
		flashcfg-debian -f $FIRM_FILE
		;;
	esac
;;
rootcfg)
	if [ $MODEL == "obsbx1" ]; then
		case $ROOT_TARGET in
		initrd|emmc|mmcblk1p*|sda*)
			if [ -x /usr/sbin/fw_printenv -a -x /usr/sbin/fw_setenv ]; then
				val=`/usr/sbin/fw_printenv rootfs` 
				if [[ "$val" == *$ROOT_TARGET ]]; then
					echo "Already rootfs $ROOT_TARGET"
				else
					case $ROOT_TARGET in
					initrd)
					fw_setenv rootfs root=/dev/ram
					;;
					emmc)
					fw_setenv rootfs root=/dev/mmcblk0p10
					;;
					*)
					fw_setenv rootfs root=/dev/$ROOT_TARGET
					;;
					esac
					echo "Change rootfs $ROOT_TARGET"
				fi
			else
				echo "This firmware is not supported this option."
			fi
		;;
		*)
			if [ ! $ROOT_TARGET ]; then
				/usr/sbin/fw_printenv rootfs | cut -d "=" -f 3
			else
				echo;echo "ERROR: unsupportted device"
				echo;
				exit 1
			fi
		;;
		esac
	else
		case $ROOT_TARGET in
		initrd|cf|ext|sda*) flashcfg-debian -c $ROOT_TARGET ;;
		*) echo;echo "ERROR: unsupportted device"; echo; exit 1;;
		esac
	fi
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
chgos)
	if [ -x /usr/sbin/fw_printenv -a -x /usr/sbin/fw_setenv ]; then
		val=`/usr/sbin/fw_printenv stdcmd` 
		if [ ! $osname ]; then
			if [[ $val == *bootYocto ]]; then
				echo "Boot Yocto"
			elif [[ $val == *bootDebian ]]; then
				echo "Boot Debian"
			else
				echo "u-boot env is old. abort!"
			fi
		elif [ "$osname" == "debian" ]; then
			if [[ $val == *bootYocto ]]; then
				fw_setenv stdcmd run bootDebian
				echo "Change boot Debian"
			elif [[ $val == *bootDebian ]]; then
				echo "Already boot Debian"
			else
				echo "u-boot env is old. abort!"
			fi
		elif [ "$osname" == "yocto" ]; then
			if [[ $val == *bootDebian ]]; then
				fw_setenv stdcmd run bootYocto
				echo "Change boot Yocto"
			elif [[ $val == *bootYocto ]]; then
				echo "Already boot Yocto"
			else
				echo "u-boot env is old. abort!"
			fi
		else
			echo "'-o' option args is debian or yocto !!"
		fi
	else
		echo "This firmware is not supported this option."
	fi
	;;
*)
	_usage
;;
esac

exit 0
