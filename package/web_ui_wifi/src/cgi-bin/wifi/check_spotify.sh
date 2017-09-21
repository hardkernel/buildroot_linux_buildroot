#!/bin/sh
ps -fe | grep librespot | grep -v grep > /dev/null
if [ $? -eq 0 ]
then
echo yesyes > spotify_status.txt
else
echo no > spotify_status.txt
fi
