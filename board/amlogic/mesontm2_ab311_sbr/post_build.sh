#!/bin/bash
# $(TARGET_DIR) is the first parameter
# This script file to change rsyslogd.conf
# audioservice can output debug log to /var/log/audioservice.log

set -x
TARGET_DIR=$1
echo "Run post build script to target dir $TARGET_DIR"

if [ -f $TARGET_DIR/etc/alsa_bsa.conf ]; then
    echo "device=dmixer_auto" > $TARGET_DIR/etc/alsa_bsa.conf
fi

if [ -f $TARGET_DIR/etc/init.d/S44bluetooth ]; then
    sed -i 's/bt_name=\"amlogic\"/bt_name=\"amlogic-TM2-SBR\"/g' $TARGET_DIR/etc/init.d/S44bluetooth
fi

if [ -d $TARGET_DIR/lib/debug ]; then
    rm -frv $TARGET_DIR/lib/debug
fi

#echo "Remove unnecessary BSA apps"
#find $TARGET_DIR/usr/bin -name app_* ! -name app_manager -delete

