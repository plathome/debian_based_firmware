#!/bin/sh

default_env(){
}

obsiot_env(){
}

usage(){
	echo "Update uboot environment for OpenBlocks TB3N"
	echo
	echo "usage: $(basename $0) [-a]"
	echo
	echo "	-a			OpenBlocks TB3N default environment"
	echo "	-h	This messages"
}

if [ "$1" = "-a" ]; then
	obsiot_env
elif [ "$1" = "--imx8mp-default" ]; then
	default_env
else
	usage
	exit 1
fi

exit 0
