#
# Regular cron jobs for the coverflowmp package.
#
0 4	* * *	root	[ -x /usr/bin/coverflowmp_maintenance ] && /usr/bin/coverflowmp_maintenance
