#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
echo $1 > ./spotify/spotify.conf
echo $2 >> ./spotify/spotify.conf
echo $3 >> ./spotify/spotify.conf
echo infos > ./spotify/pub_name

uname=$1
pwd=$2
dname=$3

#librespot -u $1 -p $2 -n $3 -c /mnt
info_data=`awk -vFS=, 'NR==FNR{split($0,a,FS);next}{split($0,b,FS);for(i in a){c[i]="\042"a[i]"\042:\042"b[i]"\042"};printf FNR==1?"[":",";$0="{"c[1]"}";printf $0}END{print "]"}' ./spotify/pub_name ./spotify/spotify.conf`
echo $info_data > ./spotify/spotify_info.txt

librespot -u $uname -p $pwd -n $dname -c /mnt
