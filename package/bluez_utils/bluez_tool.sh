#!/bin/sh

NAME1=bluetoothd
DAEMON1=/usr/sbin/$NAME1
PIDFILE1=/var/run/$NAME1.pid

Blue_start()
{
    echo 0 > /sys/class/rfkill/rfkill0/state
    sleep 1
    echo 1 > /sys/class/rfkill/rfkill0/state
    sleep 1

    echo
    echo "|-----start bluez----|"
    hciattach -s 115200 /dev/ttyS1 any
    sleep 1
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
    sleep 2
    rm -f $PIDFILE1
    killall hciattach
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

