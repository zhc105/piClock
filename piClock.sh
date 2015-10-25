#!/bin/sh
### BEGIN INIT INFO
# Provides:          piClock
# Required-Start:    $remote_fs $network
# Required-Stop:     $remote_fs $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: RaspberryPi network clock with BMP085+LCD1602 
### END INIT INFO

USER=root
ROOT=/home/pi/workspace/piClock
DAEMON=./piClock

case "$1" in
start)
    echo "Start piClock"
    umask 0002
    su - $USER -c "cd $ROOT; $DAEMON"
    ;;
stop)
    echo "Stopping piClock, please wait..."
    killall -s SIGINT -w piClock
    ;;
restart)
    echo "Stopping piClock, please wait..."
    killall -s SIGINT -w piClock
    echo "Start piClock"
    umask 0002
	cd $ROOT
    su - $USER -c "cd $ROOT; $DAEMON"
    ;;
*)
    echo "$0 {start|stop|restart|status}"
    ;;
esac
exit
