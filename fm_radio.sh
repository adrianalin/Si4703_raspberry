#!/bin/sh

### BEGIN INIT INFO
# Provides:        fm radio startup script
# Required-Start:  $network $remote_fs $syslog
# Required-Stop:   $network $remote_fs $syslog
# Default-Start:   2 3 4 5
# Default-Stop:
# Short-Description: Start fm radio
### END INIT INFO

PATH=/sbin:/bin:/usr/sbin:/usr/bin

case $1 in
        start)
		echo "Starting fm radio"
		/home/pi/fm_radio/fm_radio &
		;;
	stop)
		echo "Stopping fm radio"
		pkill -INT fm_radio
		;;
	restart)
		echo "Restarting fm radio"
		$0 stop && sleep 2 && $0 start
		;;
	*)
                echo "Usage: $0 {start|stop|restart}"
                exit 2
                ;;
esac
