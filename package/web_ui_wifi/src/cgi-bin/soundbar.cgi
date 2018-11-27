#!/bin/sh
if [ -f "/tmp/dbus/dbus-addr" ]; then
	export $(cat /tmp/dbus/dbus-addr)
fi
if [ -f "/tmp/dbus/dbus-is -pid" ]; then
    export $(cat /tmp/dbus/dbus-is -pid)
fi
export XDG_CONFIG_HOME=/var


echo -ne "Content-type:text/html\n\n"
#echo -ne "<body> </body>"

#QUERY_STRING="command=enable-input&sourceid=SPDIF"
logger -p local3.info "enter soundbar.cgi $QUERY_STRING"
logger -p local3.debug "param number $? first = $0 second $1"

query_str=$QUERY_STRING
#query_str="command=enable-input&sourceid=SPDIF"
logger -p local3.debug "query_str = $query_str"
command=${query_str%&*}
command=${command#command=}

param=${query_str#*&}
param=${param#value=}

result=$(echo $command | grep "set-eq")
if [ "$result" != "" ]; then
    index=${command#set-eq}
    command="set-eq"
fi

logger -p local3.debug "command = $command, source id = $param"
case $command in
    "getsysteminfo")
    logger -p local3.debug "get system settings"
    asplay.py showsystemsettings
    exit 0
    ;;

    "set-EQ")
    asplay.py $command $param
    ;;

    # reset eq to default
    "reset-EQ")
    logger -p local3.debug "reset EQ, update eq info"
    asplay.py reset-equalizer
    ;;

    # set eq valule
    "set-eq")
    logger -p local3.debug "set EQ, index = $index"
    asplay.py set-equalizer $index $param
    ;;

# stop player
    "dataplay-stop")
    playerpid=$(ps -ef|grep asplay.py|grep -v grep)
    playerpid=${playerpid%% *}
    logger -p local3.debug "exec kill command pid = $playerpid"
    if [ -n $playerpid ] ; then
        kill -s SIGINT $playerpid
    else
        logger -p local3.debug "there is no related player process exist"
    fi
    ;;

# start player
    "dataplay-start")
    logger -p local3.debug "start play /data/ness/test2.$param"
    asplay.py /data/ness/test2.$param &
    ;;

    "set-volume")
    asplay.py $command $param
    ;;

    "enable-input")
    asplay.py $command $param
    asplay.py showinput
    #cat /tmp/audioinput.txt
    ;;

    "set-mute")
    asplay.py $command $param
    ;;

    "showinput")
    asplay.py showinput
    #cat /tmp/audioinput.txt
    ;;

    *)
    logger -p local3.error "Unsupport command $command"
    ;;
esac

exit 0


