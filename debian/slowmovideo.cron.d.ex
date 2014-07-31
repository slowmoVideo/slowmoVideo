#
# Regular cron jobs for the slowmovideo package
#
0 4	* * *	root	[ -x /usr/bin/slowmovideo_maintenance ] && /usr/bin/slowmovideo_maintenance
