rtlwifi_new
===========

A repo for the newest Realtek rtlwifi codes.

This code will build on any kernel 3.0 and newer as long as the distro has not modified
any of the kernel APIs. It includes the following drivers:

rtl8192ce, rtl8192se, rtl8192de, rtl8188ee, rtl8192ee, rtl8723ae, rtl8723be, and rtl8821ae.

Added March 16, 2016: All branches of this repo now support the ant_sel module option
for rtl8723be. In addition, patches to implement this feature have been submitted
to the linux-wireless repo. If accepted, they should appear in kernel 4.7; however,
they will be backported to kernels 4.0 and newer when they reach mainline.

