#!/bin/sh

if [ $2 ];then
	mode=$2
else
	mode="a2dp"
fi

if [ $3 ];then
	device=$3
else
	device="rtk"
fi

echo "|--bluez: device = $device mode = $mode--|"

realtek_bt_init()
{
	modprobe rtk_btuart
	modprobe rtk_btusb
	usleep 500000
	rtk_hciattach -n -s 115200 /dev/ttyS1 rtk_h5 &
}

A2DP_service()
{
	echo "|--bluez a2dp-sink/hfp-hf service--|"
	hciconfig hci0 up
	sleep 1
	/usr/libexec/bluetooth/bluetoothd -n &
	sleep 1
	bluealsa -p a2dp-sink -p hfp-hf &
	bluealsa-aplay --profile-a2dp 00:00:00:00:00:00 -d dmixer_avs_auto &
	default_agent &
	hciconfig hci0 piscan
	hciconfig hci0 inqparms 18:1024
	hciconfig hci0 pageparms 18:1024


}

BLE_service()
{
       echo "|--bluez ble service--|"
       hciconfig hci0 up
       hciconfig hci0 noscan
       sleep 1
       btgatt-server &
}

service_down()
{
	echo "|--stop bluez service--|"
	killall default_agent
	killall bluealsa-aplay
	killall bluealsa
	killall bluetoothd
	killall btgatt-server
	hciconfig hci0 down

}

Blue_start()
{
	echo 0 > /sys/class/rfkill/rfkill0/state
	usleep 500000
	echo 1 > /sys/class/rfkill/rfkill0/state

	echo
	echo "|-----start bluez----|"
	if [ $device = "rtk" ];then
		realtek_bt_init
	else
		modprobe hci_uart
		usleep 300000
		hciattach -s 115200 /dev/ttyS1 any
	fi
	local cnt=10
	while [ $cnt -gt 0 ]; do
		hciconfig hci0 2> /dev/null
		if [ $? -eq 1 ];then
			echo "checking hci0 ......."
			sleep 1
			cnt=$((cnt - 1))
		else
			break
		fi
	done

	if [ $cnt -eq 0 ];then
		echo "hcio shows up fail!!!"
		exit 0
	fi

	if [ $mode = "ble" ];then
		BLE_service
	else
		A2DP_service
	fi

	echo "|-----bluez is ready----|"

}

Blue_stop()
{
	echo -n "Stopping bluez"
	service_down
	killall rtk_hciattach
	killall hciattach
	rmmod hci_uart
	rmmod rtk_btusb
	sleep 2
	echo 0 > /sys/class/rfkill/rfkill0/state
	echo
	echo "|-----bluez is shutdown-----|"
}

case "$1" in
	start)
		Blue_start &
		;;
	restart)
		Blue_stop
		Blue_start &
		;;
	up)
		service_up
		;;
	down)
		service_down
		;;
	reset)
		service_down
		service_up
		;;
	netready|netup|netdown|netchange) ;;
	stop)
		Blue_stop
		;;
	*)
		echo "Usage: $0 {start|stop}"
		exit 1
esac

exit $?

