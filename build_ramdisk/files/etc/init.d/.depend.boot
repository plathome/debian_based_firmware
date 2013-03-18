TARGETS = mountkernfs.sh udev mountdevsubfs.sh urandom mountall.sh mountoverflowtmp networking ifupdown hwclock.sh checkroot.sh mountnfs.sh mountnfs-bootclean.sh ifupdown-clean hostname.sh bootlogd procps hwclockfirst.sh mtab.sh checkfs.sh mountall-bootclean.sh bootmisc.sh stop-bootlogd-single module-init-tools udev-mtab obs600setup
INTERACTIVE = udev checkroot.sh checkfs.sh
udev: mountkernfs.sh obs600setup
mountdevsubfs.sh: mountkernfs.sh udev
urandom: mountall.sh mountoverflowtmp
mountall.sh: checkfs.sh
mountoverflowtmp: mountall-bootclean.sh
networking: mountkernfs.sh mountall.sh mountoverflowtmp ifupdown
ifupdown: ifupdown-clean
hwclock.sh: checkroot.sh bootlogd
checkroot.sh: mountdevsubfs.sh hostname.sh hwclockfirst.sh bootlogd
mountnfs.sh: mountall.sh mountoverflowtmp networking ifupdown
mountnfs-bootclean.sh: mountall.sh mountoverflowtmp mountnfs.sh
ifupdown-clean: checkroot.sh
hostname.sh: bootlogd
bootlogd: mountdevsubfs.sh
procps: bootlogd mountkernfs.sh mountall.sh mountoverflowtmp udev module-init-tools
hwclockfirst.sh: bootlogd mountdevsubfs.sh
mtab.sh: checkroot.sh
checkfs.sh: checkroot.sh mtab.sh
mountall-bootclean.sh: mountall.sh
bootmisc.sh: mountall.sh mountoverflowtmp mountnfs.sh mountnfs-bootclean.sh udev
stop-bootlogd-single: mountall.sh mountoverflowtmp udev urandom networking ifupdown hwclock.sh checkroot.sh mountkernfs.sh mountnfs.sh mountnfs-bootclean.sh ifupdown-clean hostname.sh mountdevsubfs.sh bootlogd procps hwclockfirst.sh mtab.sh checkfs.sh mountall-bootclean.sh bootmisc.sh module-init-tools udev-mtab
module-init-tools: checkroot.sh
udev-mtab: udev mountall.sh mountoverflowtmp
