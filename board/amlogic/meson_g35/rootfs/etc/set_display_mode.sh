#!/bin/sh
display_mode=720p 
#$(cat /sys/class/display/mode)

echo 1 > /sys/class/graphics/fb0/blank
echo 1 > /sys/class/graphics/fb1/blank

echo 720p > /sys/class/display/mode
echo 0 0 1280 720 0 > /sys/class/ppmgr/ppscaler_rect


echo 0 0 1280 720 0 0 18 18 > /sys/class/display/axis
echo 0 > /sys/class/graphics/fb0/freescale_mode
echo 0 > /sys/class/graphics/fb1/freescale_mode
echo 0 0 1279 719 > /sys/class/graphics/fb0/free_scale_axis
echo 0x10001 > /sys/class/graphics/fb0/free_scale


echo 0 > /sys/class/graphics/fb0/blank

