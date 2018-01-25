#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
WIFI_FILE=/var/www/cgi-bin/wifi/select.txt
ssid="default_amlogic"
password="default_amlogic"

parse_paras()
{
	ssid=`sed -n "1p" $WIFI_FILE`
	password=`sed -n "2p" $WIFI_FILE`
	if [ "`echo $password |wc -L`" -lt "8" ];then
		echo "waring: password lentgh is less than 8, it is not fit for WPA-PSK"
	fi
}

ping_test()
{
	killall udhcpc
        if [ $1 -eq 0 ];then
                lightTest --num=12 --times=0 --speed=300 --time=0 --style=0 --mute_led=1 --listen_led=1
                echo "ping fail!! ip is NULL"
                if [ -f "/etc/bsa/config/wifi_status" ]; then
                        echo 0 > /etc/bsa/config/wifi_status
                fi
		return 0
        fi
	echo "now going to ping router's ip: $1 for 5 seconds"
	ping $1 -w 5
	if [ $? -eq 1 ];then
		lightTest --num=12 --times=0 --speed=300 --time=0 --style=0 --mute_led=1 --listen_led=1
		echo "ping fail!! please check"
		if [ -f "/etc/bsa/config/wifi_status" ]; then
			echo 0 > /etc/bsa/config/wifi_status
		fi
	else
		echo "ping successfully"
		wpa_cli save_config
		sync
		lightTest --num=12 --times=0 --speed=150 --time=3 --style=30 --mute_led=1 --listen_led=1
		if [ -f "/etc/bsa/config/wifi_status" ] ;then
			echo 1 > /etc/bsa/config/wifi_status
		fi
		sleep 2
		lightTest --num=12 --times=0 --speed=300 --time=0 --style=0 --mute_led=1 --listen_led=1
	fi
}

check_state()
{
	local cnt=1
	while [ $cnt -lt $1 ]; do
		echo "check_in_loop processing..."
		ret=`wpa_cli status | grep "wpa_state"`
		ret=${ret##*=}
		if [ $ret == "COMPLETED" ]; then
			return 1
		else
			cnt=$((cnt + 1))
			sleep 1
			continue
		fi
        done
	return 0
}

wifi_setup()
{
	parse_paras
	wpa_cli -iwlan0 remove_network 0
	id=`wpa_cli add_network | grep -v "interface"`
	echo "***************wifi setup paras***************"
	echo "**  id=$id                                  **"
	echo "**  ssid=$ssid                              **"
	echo "**  password=$password                      **"
	echo "**********************************************"
	wpa_cli set_network $id ssid \"$ssid\"
	if [ "$password" = "NONE" ]; then
		wpa_cli set_network $id key_mgmt NONE
	else
		wpa_cli set_network $id psk \"$password\"
	fi
	wpa_cli enable_network $id
	check_state 10
	if [ $? -eq 0 ] ;then
		echo "start wpa_supplicant fail!!"
	else
		echo "start wpa_supplicant successfully!!"
		ip_addr=`udhcpc -q -n -s /usr/share/udhcpc/default.script -i wlan0 2> /dev/null | grep "adding dns*" | awk '{print $3}'`
	fi
	ping_test $ip_addr $id
}

wifi_setup
