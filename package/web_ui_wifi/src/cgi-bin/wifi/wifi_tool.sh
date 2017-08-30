#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
ssid="appolo"
password="Amluser88!!"
driver="dhd"
mode="station"
config_file="./wifi_configure.txt"
driver_list="dhd ath10k_pci"
router_ip="192.168.168.1"
router_connted="no"
ap_ip="192.168.2.1"
ping_period="4"
retry="1"
onoff_test="0"
debug="0"
ap_s400="firmware_path=/etc/wifi/6255/fw_bcm43455c0_ag_apsta.bin nvram_path=/etc/wifi/6255/nvram.txt"
ap_s420="firmware_path=/etc/wifi/4356/fw_bcm4356a2_ag_apsta.bin nvram_path=/etc/wifi/4356/nvram.txt"
station_s400="firmware_path=/etc/wifi/6255/fw_bcm43455c0_ag.bin nvram_path=/etc/wifi/6255/nvram.txt"
station_s420="firmware_path=/etc/wifi/4356/fw_bcm4356a2_ag.bin nvram_path=/etc/wifi/4356/nvram.txt"

NAME1=wpa_supplicant
DAEMON1=/usr/sbin/$NAME1
PIDFILE1=/var/run/$NAME1.pid

NAME2=hostapd
DAEMON2=/usr/sbin/$NAME2
PIDFILE2=/var/run/$NAME2.pid

NAME3=dnsmasq
DAEMON3=/usr/sbin/$NAME3
PIDFILE3=/var/run/$NAME3.pid

NAME4=dhcpcd
DAEMON4=/usr/sbin/$NAME4
PIDFILE4=/var/run/${NAME4}-wlan0.pid



############################################################################################
###############Function Zone################################################################
############################################################################################

function main() {

###########show usage first#####################
usage > /dev/null
###########initialize ssid passwd etc###########
initial_configure $1 $2 $3 $4 $5 > /dev/null

#########stop wifi first#################
stop_wifi > /dev/null

########if want to disable wifi,should exit here#
if [ "$1" = "stop" ];then
echo "wifi function stopped!" > /dev/null
end_script > /dev/null
fi

if [ $onoff_test -eq 1 ]; then
##############wifi on/off loop begin#############
	wifi_onoff_loop > /dev/null
else
########start station or ap #####################
	start_wifi > /dev/null
    end_script > /dev/null
fi
}

function start_bridge() {

echo "starting bridge.."
ifconfig eth0 0.0.0.0
ifconfig wlan0 0.0.0.0
#####create new br0##############
brctl addbr br0
brctl addif br0 eth0
#brctl addif br0 wlan0
if [ $? -eq 1 ];then
	echo "fail to add bridge"
	return;
fi
ifconfig br0 up

echo "starting br0 dhcp..."
if [ $debug -eq 1 ];then
dhcpcd br0 -t 15
else
dhcpcd br0 -t 15 > /dev/null
fi

ping_test br0
if [ "$router_connted" = "yes" ];then
	echo "bridge finish!!"
else
	echo "connet router failed"
fi
}

function start_eth() {
	ifconfig eth0 down > /dev/null
	sleep 1
	ifconfig eth0 up
	sleep 2
}


function initial_configure() {
if [ -f $config_file ];then
########load from txt##################
echo "reading from txt...."
	while read line ; do
	key=`echo $line | awk -F "=" '{print $1}'`
	val=`echo $line | awk -F "=" '{print $2}'`
    case "$key" in
		ssid)
		ssid=$val
		;;
		password)
		password=$val
		;;
		driver)
		driver=$val
		;;
		mode)
		mode=$val
		;;
		debug)
		debug=$val
		;;
		ping_period)
		ping_period=$val
		;;
		retry)
		retry=$val
		;;
        onoff_test)
		onoff_test=$val
		;;
	esac
	done < $config_file
else
########load from input################
    echo "reading from input...."
	if [ $1 ]; then
	    ssid=$1
	fi
	if [ $2 ]; then
	    password=$2
	fi
	if [ $3 ]; then
	    driver=$3
	fi
	if [ $4 ]; then
	    mode=$4
	fi
	if [ "${5}" = "debug" ]; then
    	debug="1"
	fi
fi
echo "user set:
ssid=$ssid, key=$password, driver=$driver mode=$mode debug=$debug
4s to check your configure
"
if [ "`echo $password |wc -L`" -lt "8" ];then
echo "waring: password lentgh is less than 8, it is not fit for WPA-PSK"
fi

##########disable kernel printk##################
if [ ! $debug -eq 1 ]; then
	enable_printk 0
fi
}



function load_driver() {
if [ $1 = "0" ];then
	echo "removing driver if loaded"
	local cnt=1
	driver_num=`echo $driver_list | awk -F " " '{print NF+1}'`
	while [ $cnt -lt $driver_num ]; do
		loaded_driver=`echo $driver_list | awk -F " " '{print $'$cnt'}'`
		lsmod | grep $loaded_driver
		if [ $? -eq 0 ];then
			echo "loaded_driver=$loaded_driver"
			rmmod $loaded_driver
		fi
		cnt=$((cnt + 1))
	done
else
	echo "start driver loading..."
	HW_PLATFORM=$(cat /proc/device-tree/amlogic-dt-id | awk -F "_" '{print $2}')
	if [ "$mode" == "ap" -a "$driver" == "dhd" ];then
		if [ "${HW_PLATFORM}" == "s400" ]
		then
			modprobe $driver $ap_s400
        elif [ "${HW_PLATFORM}" == "s420" ]
        then
			modprobe $driver $ap_s420
        fi

	else
		if [ "${HW_PLATFORM}" == "s400" ]
		then
			modprobe $driver $station_s400
        elif [ "${HW_PLATFORM}" == "s420" ]
        then
			modprobe $driver $station_s420
        fi
	fi

	if [ $? -eq 0 ]; then
		echo "dirver loaded"
	else
		echo "fail to load driver"
		end_script
	fi

	##########check wlan0############################
	echo "checking wlan0..."
	check_in_loop 10 check_wlan
	echo "wlan0 shows up
	"
fi
}

function stop_wifi_app() {
echo "Stopp prv wpa_supplicant first"
start-stop-daemon -K -o -p $PIDFILE1 2> /dev/null
sleep 1
echo "Stopp prv hostapd first"
start-stop-daemon -K -o -p $PIDFILE2 2> /dev/null
sleep 1
echo "Stopp prv dnsmasq first"
start-stop-daemon -K -o -p $PIDFILE3 2> /dev/null
sleep 1
echo "Stopp prv dhcpcd first"
start-stop-daemon -K -o -p $PIDFILE4 2> /dev/null
sleep 1
echo "delete prv br0"
ifconfig | grep br0 > /dev/null
if [ $? -eq 0 ];then
	ifconfig br0 down > /dev/null
	brctl delbr br0
fi
sleep 1

}

function usage() {
echo "
##################################################################
#usage:
#first choice:
#   write configure in /etc/wifi_configure.txt
#second choice:
#   $0  \"ssid\" \"key\" \"driver\" \"mode\"
#   example:$0 $ssid $password $driver $mode
#   dirver choice: dhd; ath10k. default to dhd
#   version:1.4
##################################################################
"
}

function enable_printk() {
if [ "${1}" = "1" ];then
	echo 7 > /proc/sys/kernel/printk
elif [ "${1}" = "0" ];then
	echo 1 > /proc/sys/kernel/printk
fi
}

function end_script() {
if [ ! $debug -eq 1 ];then
	enable_printk 1
fi
exit
}
alias check_wlan="ifconfig wlan0 2> /dev/null"
alias check_wpa="wpa_cli ping 2> /dev/null | grep PONG"
alias check_ap_connect="wpa_cli status 2> /dev/null | grep state=COMPLETED"
alias check_hostapd="hostapd_cli status 2> /dev/null | grep state=ENABLED"
alias check_dnsmasq="ps | grep -v grep | grep dnsmasq > /dev/null"

function check_in_loop() {
local cnt=1
while [ $cnt -lt $1 ]; do
    echo "check_in_loop processing..."
    case "$2" in
        check_wlan)
        check_wlan
        ;;
        check_hostapd)
        check_hostapd
        ;;
        check_dnsmasq)
        check_dnsmasq
        ;;
        check_wpa)
        check_wpa
        ;;
        check_ap_connect)
        check_ap_connect
        ;;
    esac
    if [ $? -eq 0 ];then
        return
    else
        cnt=$((cnt + 1))
        sleep 1
        continue
	fi
done
##return here if no matter###
if [ "$2" = "check_eth" ];then
	return
fi

echo "fail!!"
end_script
}

function wifi_onoff_loop() {
echo "
#####################################################
#####begin to turn on/off wifi for $retry times
#####################################################
"
sleep 1
local cnt=0
while [ $cnt -lt $retry ]; do
	start_wifi
	sleep 2
	stop_wifi
	sleep 5
	cnt=$((cnt + 1))
	echo "wifi has been tuned on/off for $cnt times...
    "
done
echo "wifi on/off test passed!!"
end_script
}

function stop_wifi() {

echo "#########stoping wifi#####################"
#####stop wpa_supplicant hostapd dhcpcd dnsamas##
stop_wifi_app
#########remove all loaded wifi driver###########
load_driver 0
}

function start_wifi() {

echo "########starting wifi#####################"
###############load wifi driver##################
load_driver 1
#####start wpa_supplicant hostapd dhcpcd dnsamasq bridge##

if [ "${mode}" = "station" ]; then
	start_sta
	ping_test wlan0
elif [ "${mode}" = "ap" ]; then
	start_eth
	start_bridge
	start_ap
###if eth0 is conneted to route, we start add wlan0 into bridge####
###otherwise we use dnsmasq########################################
    if [ "$router_connted" = "yes" ];then
		brctl addif br0 wlan0
	else
		start_dnsmasq
	fi
    echo "ap is started!!"
else
	echo "bad mode!"
	end_script
fi

}


#########start hostapd###################
function start_ap() {

#create hostapd configure
echo "starting hostapd..."
echo "interface=wlan0
driver=nl80211
ctrl_interface=/var/run/hostapd
ssid=${ssid}
channel=6
ieee80211n=1
hw_mode=g
ignore_broadcast_ssid=0"  > /etc/hostapd_temp.conf

if [ ! "${password}" = "NONE" ];then
    echo "
wpa=3
wpa_passphrase=${password}
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP CCMP
rsn_pairwise=CCMP" >> /etc/hostapd_temp.conf
fi
if [ $debug -eq 1 ];then
    start-stop-daemon -S  -m -p $PIDFILE2  -x $DAEMON2 -- /etc/hostapd_temp.conf -B -P /var/run/hostapd.pid
else
    start-stop-daemon -S -b -m -p $PIDFILE2  -x $DAEMON2 -- /etc/hostapd_temp.conf
fi

check_in_loop 6 check_hostapd
echo "start hostpad successfully!!
"
##remove temp conf if debug is off##########
if [ ! $debug -eq 1 ];then
	rm /etc/hostapd_temp.conf
fi
}

###############start dnsmasq#################
function start_dnsmasq() {
echo "starting dnsmasq..."
ifconfig wlan0 $ap_ip
echo "ap_ip=$ap_ip"

start-stop-daemon -S -m -p $PIDFILE3 -b -x $DAEMON3  -- -iwlan0  --dhcp-option=3,${ap_ip} --dhcp-range=${ap_ip%.*}.50,${ap_ip%.*}.200,12h -p100

check_in_loop 6 check_dnsmasq
echo "start dnsmasq successfully!!"
}

############start wpa_supplicant##########
function start_sta() {
echo "starting wpa_supplicant..."
ifconfig wlan0 0.0.0.0

if [ $debug -eq 1 ];then
	start-stop-daemon -S -m -p $PIDFILE1 -x $DAEMON1 -- -Dnl80211 -iwlan0 -c/etc/wpa_supplicant.conf -d -B -P $PIDFILE1
else
	ap_ssid_num=`cat /etc/wifi/ap_name`
	ap_ssid=amlogic_audio_$ap_ssid_num
	dhd_priv iapsta_init mode apsta
	dhd_priv iapsta_config ifname wlan1 ssid $ap_ssid chan 6 amode open emode none
	dhd_priv iapsta_enable ifname wlan1
	ifconfig wlan1 192.168.2.1
	start-stop-daemon -S -m -p $PIDFILE3  -x $DAEMON3  -- -iwlan1  --dhcp-option=3,192.168.2.1 --dhcp-range=192.168.2.50,192.168.2.200,12h -p100
	start-stop-daemon -S -m -p $PIDFILE1 -b -x $DAEMON1 -- -Dnl80211 -iwlan0 -c/etc/wpa_supplicant.conf
fi
check_in_loop 10 check_wpa
echo "connecting ap ...."
id=`wpa_cli add_network | grep -v "interface"`
wpa_cli set_network $id ssid \"${ssid}\" > /dev/null
if [ "$password" = "NONE" ]; then
    wpa_cli set_network $id key_mgmt NONE
else
    wpa_cli set_network $id psk \"${password}\" > /dev/null
fi
wpa_cli select_network $id  > /dev/null
wpa_cli enable_network $id  > /dev/null
wpa_cli save_config

check_in_loop 10 check_ap_connect
echo "start wpa_supplicant successfully!!"

############start dhcp#######################
echo "starting wifi dhcp..."
if [ $debug -eq 1 ];then
dhcpcd wlan0
else
dhcpcd wlan0 > /dev/null
fi
echo "ap connected!!"
}

function ping_test() {
router_ip=`dhcpcd -U $1 2> /dev/null | grep routers | awk -F "=" '{print $2}' | sed "s/'//g"`
echo "
now going to ping router's ip: $router_ip for $ping_period seconds"
ping $router_ip -w $ping_period
if [ $? -eq 1 ];then
echo "ping fail!! please check"
else
echo "ping successfully"
router_connted="yes"
fi
}
main $1 $2 $3 $4 $5
