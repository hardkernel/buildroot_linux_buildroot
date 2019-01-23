#!/bin/sh
# $(TARGET_DIR) is the first parameter
# This script file to change rsyslogd.conf
# audioservice can output debug log to /var/log/audioservice.log

# It's used to change /etc/asound.conf
# set pcm.!default and ctl.!default to pulse directly
echo "Start to change /etc/rsyslog.conf to enable audioservice log"
if [ -f $1/etc/rsyslog.conf ] ; then
# audioservice uses syslog local2
# we will set it's log level with info
	textexist=$(cat $1/etc/rsyslog.conf | grep local2)
	# echo "textexist = $textexist"
	if [ -z "$textexist" ] ; then
		sed -i '/local7.*/alocal2.info\t\t\/dev\/ttyS0' \
			$1/etc/rsyslog.conf
	fi

# asr_uartcmd uses syslog local3
# we will set it's log level with *
#	textexist=$(cat $1/etc/rsyslog.conf | grep local3)
#	# echo "textexist = $textexist"
#	if [ -z "$textexist" ] ; then
#		sed -i '/local7.*/alocal3.*\t\t\t\/var\/log\/audioservice.log' \
#			$1/etc/rsyslog.conf
#	fi
fi

# Change the ALSA device for BT
if [ -f $1/etc/alsa_bsa.conf ] ; then
	textexist=$(cat $1/etc/alsa_bsa.conf | grep 2to8)
	# echo "textexist = $textexist"
	if [ -z "$textexist" ] ; then
		sed -i 's/dmixer_avs_auto/2to8/g' $1/etc/alsa_bsa.conf
	fi
fi

# MCU6350 gpio interrupt config
echo "Start to change /etc/init.d/S90audioservice to support mcu6350"
if [ -f $1/etc/init.d/S90audioservice ] ; then
	sed -i '/#!\/bin\/sh/a\
\n# MCU6350 gpio interrupt config\
# gpio_z4(4) + base(411) = 415\
GPIO_INDEX=415\
NODE_GPIO=/sys/class/gpio/gpio$GPIO_INDEX\
if [ ! -d "$NODE_GPIO" ]; then\
	echo $GPIO_INDEX > /sys/class/gpio/export\
	echo in     > $NODE_GPIO/direction\
	echo rising > $NODE_GPIO/edge\
fi\
' $1/etc/init.d/S90audioservice
fi

# Copy related aml_halaudio configure file
# now we use the 5.1.2 8 channels config
if [ -f $1/etc/halaudio/8ch_aml_audio_config.json ] ; then
	cp $1/etc/halaudio/8ch_aml_audio_config.json \
		$1/etc/aml_audio_config.json
fi

