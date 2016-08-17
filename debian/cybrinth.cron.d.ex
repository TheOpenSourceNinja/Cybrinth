#
# Regular cron jobs for the cybrinth package
#
0 4	* * *	root	[ -x /usr/bin/cybrinth_maintenance ] && /usr/bin/cybrinth_maintenance
