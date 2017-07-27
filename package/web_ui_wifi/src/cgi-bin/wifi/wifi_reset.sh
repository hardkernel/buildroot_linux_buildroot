#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
echo "Content-type: text/html"
echo "<html>"
echo "<body>"

rm /etc/wpa_supplicant.conf
touch /etc/wpa_supplicant.conf

echo -e "ctrl_interface=/var/run/wpa_supplicant" > /etc/wpa_supplicant.conf
echo -e "ap_scan=1" >> /etc/wpa_supplicant.conf
echo -e "update_config=1" >> /etc/wpa_supplicant.conf
echo -e "\n" >> /etc/wpa_supplicant.conf
echo -e "network={" >> /etc/wpa_supplicant.conf
echo -e "   key_mgmt=NONE" >> /etc/wpa_supplicant.conf
echo -e "}" >> /etc/wpa_supplicant.conf

echo "<script>"
echo 'window.alert("now");'
echo "<script>"
echo "</body>"
echo "</html>"
