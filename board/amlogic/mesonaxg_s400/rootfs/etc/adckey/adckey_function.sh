#!/bin/sh

NAME1=app_ble_wifi_setup
DAEMON1=/usr/bin/$NAME1
PIDFILE1=/var/run/$NAME1.pid

NAME2=app_musicBox
DAEMON2=/usr/bin/$NAME2
PIDFILE2=/var/run/$NAME2.pid

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

ble_wifi_setup()
{
	echo "ble config for wifisetup"
	rm /etc/bsa/config/wifi_tool.sh
	ln /var/www/cgi-bin/wifi/wifi_tool.sh  /etc/bsa/config/wifi_tool.sh
	local app1_id=`ps | grep "app_musicBox" | awk '{print $1}'`
	kill -9 $app1_id
	local app2_id=`ps | grep "app_ble_wifi_setup" | awk '{print $1}'`
	kill -9 $app2_id
	cd /etc/bsa/config
        app_ble_wifi_setup &
	app_musicBox  ble_mode &
}

case $1 in
    "power") powerStateChange ;;
    "VolumeUp") volumeUpAction ;;
    "VolumeDown") volumeDownAction ;;
    "longpressWifiConfig") wifiSmartConfig ;;
    "WifiConfig")  ble_wifi_setup ;;
    *) echo "no function to add this case: $1" ;;
esac

exit
