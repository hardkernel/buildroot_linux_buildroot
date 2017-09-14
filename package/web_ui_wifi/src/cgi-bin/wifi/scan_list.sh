#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
wpa_cli  scan > /dev/null
wpa_cli  scan_result | awk '{print $5}' > data_wifi_list
sed -i '1,2d' data_wifi_list
echo ssid > pub_name
data=`awk -vFS=, 'NR==FNR{split($0,a,FS);next}{split($0,b,FS);for(i in a){c[i]="\042"a[i]"\042:\042"b[i]"\042"};printf FNR==1?"[":",";$0="{"c[1]"}";printf $0}END{print "]"}' pub_name data_wifi_list`

echo $data > wifi_list.txt
