#!/bin/sh

CONFIG=/etc/AlexaClientSDKConfig.json
PROCESS=/usr/bin/SampleApp
DISPLAYCARD=/sbin/DisplayCardsD

start_avs() {
    if [ -f $CONFIG ];then
        cd /usr/bin/
        ./SampleApp $CONFIG NONE back &
        cd -
    else
        echo "avs start fail: not found config file $CONFIG!!!"
    fi
}

start_speaker_process () {
    /usr/bin/speaker_process &
}

modprobe snd-aloop

amixer_ctr=`amixer controls | grep "Loopback Enable"` > /dev/null
loop_id=${amixer_ctr%%,*}
amixer cset $loop_id 1

amixer_ctr=`amixer controls | grep "datain_datalb_total"` > /dev/null
datalb_id=${amixer_ctr%%,*}
amixer cset $datalb_id 16

if [ -f $DISPLAYCARD ]; then
    $DISPLAYCARD -r 270 &
else
    echo "displaycard start fail!"
fi

if [ -r /etc/last_date ]; then
    saved_date=$(cat /etc/last_date)
    date -s "$saved_date" > /dev/null
    if [ $? = 0 ]; then
        echo "Set with last saved date $saved_date OK"
    fi
fi

while true ; do
    ps -fe|grep speaker_process |grep -v grep 1>/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "start speaker_process....."
        start_speaker_process
    fi

    ps -fe|grep SampleApp |grep -v grep 1>/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "start avs....."
        start_avs
    fi
    sleep 2
done
