#!/bin/sh
bt=$2

NAME1=bluetoothd
DAEMON1=/usr/sbin/$NAME1
PIDFILE1=/var/run/$NAME1.pid

NAME2=rtk_hciattach
DAEMON2=/usr/sbin/$NAME2
PIDFILE2=/var/run/$NAME2.pid

realtek_bt_init()
{
	modprobe rtk_btuart
	modprobe rtk_btusb
	sleep 1
	start-stop-daemon -S -b -m -p $PIDFILE2 -x $DAEMON2 -- -n -s 115200 /dev/ttyS1 rtk_h5
}

Blue_start()
{
	echo 0 > /sys/class/rfkill/rfkill0/state
	sleep 1
	echo 1 > /sys/class/rfkill/rfkill0/state
	sleep 1

	echo
	echo "|-----start bluez----|"
	if [ $bt = "rtk" ];then
		realtek_bt_init
	else
		modprobe hci_uart
		hciattach -s 115200 /dev/ttyS1 any
	fi
	sleep 1
	hciconfig hci0
	if [ $? -eq 1 ];then
		echo
		echo "hci0 not show up, bluez start fail!!"
		exit 1
	fi
	hciconfig hci0 up
	hciconfig hci0 piscan

	start-stop-daemon -S  -m -p $PIDFILE1 -x $DAEMON1 -- -n &
	sleep 1
	agent 0000 &
	echo "|-----bluez is ready----|"

}

Blue_stop()
{
	echo -n "Stopping bluez"
	start-stop-daemon -K -o -p $PIDFILE1
	start-stop-daemon -K -o -p $PIDFILE2
	sleep 2
	rm -f $PIDFILE1
	rm -f $PIDFILE2
	killall hciattach
	rmmod hci_uart
	rmmod rtk_btusb
	sleep 2
	echo 0 > /sys/class/rfkill/rfkill0/state
	echo "


|-----bluez is shutdown-----|"
}

case "$1" in
	start)
		Blue_start &
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
