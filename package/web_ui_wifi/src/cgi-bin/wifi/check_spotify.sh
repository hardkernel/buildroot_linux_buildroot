#!/bin/sh
ps -fe | grep librespot | grep -v grep > /dev/null
if [ $? -eq 0 ]
then
echo "I am OK" > spotify_state.txt
#echo haha
else
echo no > spotify_state.txt
#echo wuwu
fi
