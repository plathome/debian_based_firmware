#!/bin/sh


echo 'starting...'
STAGING_DRIVER="/lib/modules/`uname -r`/kernel/drivers/staging/"

#remove staging driver
test -d $STAGING_DRIVER"rtl8821ae" && echo "will remove old rtl8821ae";rm -rf $STAGING_DRIVER"rtl8821ae"
test -d $STAGING_DRIVER"rtl8188ee" && echo "will remove old rtl8188ee";rm -rf $STAGING_DRIVER"rtl8188ee"
test -d $STAGING_DRIVER"rtl8192ce" && echo "will remove old rtl8192ce";rm -rf $STAGING_DRIVER"rtl8192ce"
test -d $STAGING_DRIVER"rtl8192de" && echo "will remove old rtl8192de";rm -rf $STAGING_DRIVER"rtl8192de"
test -d $STAGING_DRIVER"rtl8192ee" && echo "will remove old rtl8192ee";rm -rf $STAGING_DRIVER"rtl8192ee"
test -d $STAGING_DRIVER"rtl8192e" && echo "will remove old rtl8192e";rm -rf $STAGING_DRIVER"rtl8192e"
test -d $STAGING_DRIVER"rtl8192se" && echo "will remove old rtl8192se";rm -rf $STAGING_DRIVER"rtl8192se"
test -d $STAGING_DRIVER"rtl8723ae" && echo "will remove old rtl8723ae";rm -rf $STAGING_DRIVER"rtl8723ae"
test -d $STAGING_DRIVER"rtl8723be" && echo "will remove old rtl8723be";rm -rf $STAGING_DRIVER"rtl8723be"
echo 'end'
