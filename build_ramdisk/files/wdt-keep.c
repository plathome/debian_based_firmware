/*  $ssdlinux: wdt-keep.c,v 0.01 2014/01/07 07:19:59 yamagata Exp $ */
/*
 * Copyright (c) 2009-2016 Plat'Home CO., LTD.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Plat'Home CO., LTD. nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <limits.h>
#include <errno.h>

#define WD_FILE		"/dev/watchdog"
#define PID_FILE	"/var/run/watchdog.pid"
#if defined(CONFIG_OBSA7) || defined(CONFIG_OBSA6)
#define DEF_TIMEOUT		21
#define DEF_INTERVAL	10
#define MAX_TIMEOUT		21
#else
#define DEF_TIMEOUT		60
#define DEF_INTERVAL	30
#define MAX_TIMEOUT		171
#endif

static int forever;

void die(int sig)
{
	forever=0;
}

void usage(char *fname)
{
	printf("%s timeout keepalive\n", fname);
	printf("timeout\t: watchdog timer timeout 1 - %d sec\n", MAX_TIMEOUT);
	printf("keepalive\t: keepalive interval\n");
	printf("exp.\n");
	printf("watchdog timeout = %d sec\n", DEF_TIMEOUT);
	printf("keepalive interval = %d sec\n", DEF_INTERVAL);
	printf("%s %d %d\n", fname, DEF_TIMEOUT, DEF_INTERVAL);
}

int main(int ac, char* av[])
{
	char buf[100], *endp;
	int pid;
	int fd, fd2;
	long ret;
	time_t interval;
	int tout;
	struct timespec req, rem;

	if(ac != 3){
		usage(av[0]);
		exit(EXIT_FAILURE);
	}

	if((pid = fork())){
		/* parent */
		if((fd2 = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) < 0){
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}
		sprintf(buf, "%d\n", pid);
		if(write(fd2, buf, strlen(buf)) != strlen(buf)){
			close(fd2);
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}
		close(fd2);
	}
	else{
#if !defined(DEBUG)
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		if(setsid() == -1){
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}

		signal(SIGTERM, die);
		if((fd = open(WD_FILE, O_WRONLY)) == -1){
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}

		/* set watchdog timer timeout */
		tout = strtol(av[1], &endp, 10);
		if((errno == ERANGE && (tout == LONG_MAX || tout == LONG_MIN)) 
						|| (errno != 0 && tout == 0) || endp == av[1]){
			close(fd);
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}
		if(tout < 1 || tout > MAX_TIMEOUT){
			close(fd);
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: timeout is out of range\n", __LINE__);
			closelog();
			exit(EXIT_FAILURE);
		}
		if(ioctl(fd, WDIOC_SETTIMEOUT, (int*)&tout) == -1){
			close(fd);
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}

		/* set keepalive interval */
		interval = strtol(av[2], &endp, 10);
		if((errno == ERANGE && (interval == LONG_MAX || interval == LONG_MIN)) 
						|| (errno != 0 && interval == 0) || endp == av[2]){
			close(fd);
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
			closelog();
			exit(EXIT_FAILURE);
		}
		if(interval < 1 || interval > MAX_TIMEOUT){
			close(fd);
			openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
			syslog(LOG_ERR, "%d: interval is out of range\n", __LINE__);
			closelog();
			exit(EXIT_FAILURE);
		}

		forever=1;
		while(forever){
			if(ioctl(fd, WDIOC_KEEPALIVE, 0) == -1){
				openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
				syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
				closelog();
				break;
			}
			req.tv_sec = interval;
			req.tv_nsec = 0;
			for(;forever;){
				ret = nanosleep(&req, &rem);
				if(!ret){	/* normal end */
					break;
				}
				else if(ret != EINTR){
					openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
					syslog(LOG_DEBUG, "%d: nanosleep() errno=%d\n",
															__LINE__, errno);
					closelog();
					forever=0;
					break;
				}
				else{	/* ret == EINTR */
					openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
					syslog(LOG_DEBUG, "%d: nanosleep() receive EINTR",
															__LINE__);
					closelog();
					req.tv_sec = rem.tv_sec;
					req.tv_nsec = rem.tv_nsec;
				}
			}
		}
		write(fd, "V", 1);
		close(fd);
		openlog("watchdog", LOG_CONS|LOG_PID, LOG_DAEMON);
		syslog(LOG_DEBUG, "%d: loop exit\n", __LINE__);
		closelog();
	}

	exit(EXIT_SUCCESS);
}
