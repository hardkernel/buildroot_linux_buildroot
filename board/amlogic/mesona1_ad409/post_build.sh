#!/bin/sh
# $(TARGET_DIR) is the first parameter
# This script file to change rsyslogd.conf
# audioservice can output debug log to /var/log/audioservice.log

TARGET_DIR=$1

echo "Replace ff400000.dwc2_a with ff500000.dwc2_a in $TARGET_DIR/etc/init.d/S89usbgadget"
#USB hw is changed in A1
if [ -f $TARGET_DIR/etc/init.d/S89usbgadget ] ; then
    sed 's@ff400000.dwc2_a@ff500000.dwc2_a@' -i $TARGET_DIR/etc/init.d/S89usbgadget
fi

