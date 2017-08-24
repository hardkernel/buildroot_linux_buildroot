#!/bin/sh

wifiModeFlag="/etc/wifi/wifi_station"
powerStateFile="/sys/power/state"
powerResumeFlag="/etc/adckey/powerState"

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
    if [ -f $powerResumeFlag ];then
        rm $powerResumeFlag
    else
        echo "mem" > $powerStateFile
        touch $powerResumeFlag
    fi
}

volumeUpAction()
{
    local volumeMax=`amixer sget "5707_A Master"|grep "Limits:"|awk '{print $4}'`
    local volumeCurrent=`amixer sget "5707_A Master" |grep "Mono:" |awk '{print $2}'`
    if [ $volumeCurrent -le $volumeMax ];then
        let volumeCurrent+=10
        echo "$volumeCurrent"
        if [ $volumeCurrent -ge $volumeMax ];then
            volumeCurrent=$volumeMax
        fi
        amixer sset "5707_A Master" $volumeCurrent
        amixer sset "5707_B Master" $volumeCurrent
    fi
}

volumeDownAction()
{
    local volumeMin=`amixer sget "5707_A Master" |grep "Limits:" |awk '{print $2}'`
    local volumeCurrent=`amixer sget "5707_A Master" |grep "Mono:" |awk '{print $2}'`
    if [ $volumeCurrent -ge $volumeMin ];then
        let volumeCurrent-=10
        if [ $volumeCurrent -lt $volumeMin ];then
            volumeCurrent=$volumeMin
        fi
        amixer sset "5707_A Master" $volumeCurrent
        amixer sset "5707_B Master" $volumeCurrent
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
