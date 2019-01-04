#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin

if [ $1 == "start" ];then
  if [ -f "/proc/inand" ]; then
    swupdate -r -l 6 -k /etc/swupdate-public.pem -w "-document_root /var/www/swupdate/" > /tmp/swupdate.log &
  else
    swupdate -r -l 6 -k /etc/swupdate-public.pem -b "0 1 2 3 4" -w "-document_root /var/www/swupdate/" > /tmp/swupdate.log &
  fi
else
killall swupdate
fi
