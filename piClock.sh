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
DAEMON=/home/pi/workspace/piClock/piClock

case "$1" in
start)
    echo "Start piClock"
    umask 0002
    su - $USER -c "$DAEMON"
    ;;
stop)
    echo "Stopping piClock, please wait..."
    killall -w piClock
    ;;
restart)
    echo "Stopping piClock, please wait..."
    killall -w piClock
    echo "Start piClock"
    umask 0002
    su - $USER -c "$DAEMON"
    ;;
*)
    echo "$0 {start|stop|restart|status}"
    ;;
esac
exit
