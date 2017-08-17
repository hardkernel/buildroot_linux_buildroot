#!/bin/sh

wifiModeFlag="/etc/wifi/wifi_station"
powerStateFile="/sys/power/state"

wifiChangeToApMode()
{
    if [ ! -f $wifiModeFlag ];then
        echo "no found file : $wifiModeFlag"
    else
        rm /etc/wpa_supplicant.conf
        cp /etc/wpa_supplicant.conf-orig /etc/wpa_supplicant.conf
        sync
        sh /etc/init.d/S42wifi stop
        rm $wifiModeFlag
        sh /etc/init.d/S42wifi start
    fi
}

powerStateChange()
{
    echo "mem" > $powerStateFile
}

volumeUpAction()
{
    local volumeMax=`amixer sget "Master"|grep "Limits:"|awk '{print $4}'`
    local volumeCurrent=`amixer sget "Master" |grep "Mono:" |awk '{print $2}'`
    if [ $volumeCurrent -le $volumeMax ];then
        let volumeCurrent+=10
        echo "$volumeCurrent"
        if [ $volumeCurrent -ge $volumeMax ];then
            volumeCurrent=$volumeMax
        fi
        amixer sset "Master" $volumeCurrent
    fi
}

volumeDownAction()
{
    local volumeMin=`amixer sget "Master" |grep "Limits:" |awk '{print $2}'`
    local volumeCurrent=`amixer sget "Master" |grep "Mono:" |awk '{print $2}'`
    if [ $volumeCurrent -ge $volumeMin ];then
        let volumeCurrent-=10
        if [ $volumeCurrent -lt $volumeMin ];then
            volumeCurrent=$volumeMin
        fi
        amixer sset "Master" $volumeCurrent
    fi
}

case $1 in
    "longpressWifiConfig") wifiChangeToApMode ;;
    "power") powerStateChange ;;
    "VolumeUp") volumeUpAction ;;
    "VolumeDown") volumeDownAction ;;
    *) echo "no function to add this case: $1" ;;
esac

exit
