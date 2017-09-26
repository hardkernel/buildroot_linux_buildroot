#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin

if [ $1 == "start" ];then
swupdate  -w "-document_root /var/www/swupdate/" &
else
killall swupdate
fi
