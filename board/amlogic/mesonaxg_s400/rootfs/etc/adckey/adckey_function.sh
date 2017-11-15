#!/bin/sh

powerStateFile="/sys/power/state"
powerResumeFlag="/etc/adckey/powerState"

powerStateChange()
{
        echo "mem" > $powerStateFile
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

wifiSmartConfig()
{
	brcm_smartconfig.sh
}

case $1 in
    "power") powerStateChange ;;
    "VolumeUp") volumeUpAction ;;
    "VolumeDown") volumeDownAction ;;
    "longpressWifiConfig") wifiSmartConfig ;;
    *) echo "no function to add this case: $1" ;;
esac

exit
