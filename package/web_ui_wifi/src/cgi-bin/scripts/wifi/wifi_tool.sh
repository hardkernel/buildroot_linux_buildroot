#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin

WIFI_FILE=./wifi/select.txt

function main() {
	initial_configure
	start_wifi
}

function initial_configure() {
	lightTest --num=12 --times=0 --speed=150 --time=0 --style=1 --mute_led=1 --listen_led=1
	ssid=`sed -n "1p" $WIFI_FILE`
	password=`sed -n "2p" $WIFI_FILE`
	echo "$ssid"
	echo "$password"
	echo "user set:ssid=$ssid, key=$password, 4s to check your configure"
	if [ "`echo $password |wc -L`" -lt "8" ];then
		echo "waring: password lentgh is less than 8, it is not fit for WPA-PSK"
	fi
}

function start_wifi() {
	start_sta
	ping_test wlan0
}

function start_sta() {
	id=`wpa_cli add_network | grep -v "interface"`
	wpa_cli set_network $id ssid \""${ssid}"\"
	if [ "$password" = "NONE" ]; then
		wpa_cli set_network $id key_mgmt NONE
	else
		wpa_cli set_network $id psk \""${password}"\"
	fi
	wpa_cli select_network $id
	wpa_cli enable_network $id
	wpa_cli save_config

	check_in_loop 10
	echo "start wpa_supplicant successfully!!"

	dhcpcd wlan0
}


function check_in_loop() {
	local cnt=1
	while [ $cnt -lt $1 ]; do
		echo "check_in_loop processing..."
		wpa_cli status 2 | grep state=COMPLETED
		if [ $? -eq 0 ];then
			return
		else
			cnt=$((cnt + 1))
        sleep 1
        continue
		fi
	done
}


function ping_test() {
	killall dhcpcd
	router_ip=`dhcpcd -U $1 2> /dev/null | grep routers | awk -F "=" '{print $2}' | sed "s/'//g"`
	echo "now going to ping router's ip: $router_ip for 5 seconds"
	ping $router_ip -w 5
	if [ $? -eq 1 ];then
		lightTest --num=12 --times=0 --speed=300 --time=0 --style=0 --mute_led=1 --listen_led=1
		echo "ping fail!! please check"
	else
		echo "ping successfully"
		lightTest --num=12 --times=0 --speed=150 --time=0 --style=30 --mute_led=1 --listen_led=1
		sleep 2
		lightTest --num=12 --times=0 --speed=300 --time=0 --style=0 --mute_led=1 --listen_led=1
	fi
}

main
