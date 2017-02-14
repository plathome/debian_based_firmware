#!/bin/sh
#
# Update the year of the copyright
#

WRKDIR=$(cd $(dirname $0)/..; pwd)
REGEXP="Copyright.*Plat'Home CO\., LTD\."

usage()
{
echo "Usage: $0 old new [trial|update]"
echo "  old: The old year of the copyright."
echo "  new: The new year of the copyright."
echo "  trial: Show files to be updated."
echo "  update: Update the year of the copyright."
exit 1
}

if [ $# != 3 ]
then
	usage
fi

files=$(grep -l -r "${REGEXP}" ${WRKDIR}/LICENCE ${WRKDIR}/README.md ${WRKDIR}/build_ramdisk)

case $3 in
	trial)
		for p in ${files}
		do
			echo -n "$p: "
			sed -e "/${REGEXP}/s/$1/$2/g" $p | grep "${REGEXP}"
		done
		;;
	update)
		for p in ${files}
		do
			sed -e "/${REGEXP}/s/$1/$2/g" $p \
				> ${p}.tmp
			mv $p.tmp $p
			pext="${p##*.}"
			if [ x"$pext" = x"c" -o x"$pext" = x"new" ]
			then
				chmod 644 $p
			else
				chmod 755 $p
			fi
		done
		chmod 644 ${WRKDIR}/LICENCE ${WRKDIR}/README.md
		;;
	*)
		usage
		;;
esac
