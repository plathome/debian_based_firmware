#!/bin/sh

#
# Crashlog script
#
# Copyright (c) 2014, Intel Corporation.
# Simon Desfarges <simonx.desfarges@intel.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

#
# This script automatically gather kernel logs in case of crashing.
# At each boot, the script is launched by systemd at startup.
# It detects the boot reason and in case of watchdog reboot 
# (saying the platform hanged) the script will save the kernel 
# log into a file named crashlog_xxxx.
#

ipanic_console_path=/proc/emmc_ipanic_console
#crashlog_path=/home/root/
crashlog_path=/root/

# line containing 'WAKESRC' looks like:
# '[    0.530153] [BOOT] WAKESRC=[real reset] (osnib)'
# wakesrc is the 6th field with [ and ] separators
# List of available wake sources is in driver/platform/x86/intel_scu_ipcutil.c

wakesrc=`dmesg | grep WAKESRC | awk -F'[][]' '{print $6}'`

# any watchdog boot implies a crash
tmp=`echo -n "${wakesrc}" | grep watchdog`
if [ -n "${tmp}" ]; then
	# get the last sequence number (ie for crashlog_00001, get the 1)
	last_file_sequence_number=`ls ${crashlog_path}/crashlog_* | tail -1 | awk -F_ '{print $NF}'`
	if [ -z $last_file_sequence_number ]; then
		last_file_sequence_number="0"
	fi

	new_file_sequence_number=`expr ${last_file_sequence_number} + 1`
	new_file_name=`printf "${crashlog_path}/crashlog_%05d\n" $new_file_sequence_number`
	echo "****** Wake Source is [ ${wakesrc} ] ******" > ${new_file_name}

	if [ -e ${ipanic_console_path} ]; then
		# we got an epanic trace - standard case
		cat ${ipanic_console_path} >> ${new_file_name}
		echo clear > ${ipanic_console_path}
	fi
fi

