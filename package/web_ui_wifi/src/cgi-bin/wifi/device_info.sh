#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
uname -a | awk '{printf $3}' > device_info

wpa_cli  status | awk -F "=" '{print $2}' >> device_info
uname -a | awk '{printf $13}' >> device_info
sed -i '2,3d' device_info
sed -i '3d' device_info
sed -i '4d' device_info

echo info > pub_name

data=`awk -vFS=, 'NR==FNR{split($0,a,FS);next}{split($0,b,FS);for(i in a){c[i]="\042"a[i]"\042:\042"b[i]"\042"};printf FNR==1?"[":",";$0="{"c[1]"}";printf $0}END{print "]"}' pub_name device_info`
echo $data > device_info.txt
