#!/bin/sh

ssid=$1
psk=$2

sed -i '6,$d' wifi_configure.txt
echo "ssid=${ssid}" >> wifi_configure.txt
echo "password=${psk}" >> wifi_configure.txt
sh ./wifi_tool.sh > /etc/null

exit 0
