//#define DEBUG
/*  $ssdlinux: wd_keepalive.c,v 0.01 2014/01/07 07:19:59 yamagata Exp $ */
/*
 * Copyright (c) 2009-2022 Plat'Home CO., LTD.
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include "libaxio.h"

extern char *optarg;

#define PID_FILE	"/var/run/obshx1-wdt.pid"
unsigned int timeout = 60;
unsigned int interval = 10;

int forever;

void die(int sig)
{
	forever=0;
}

int sleep_interval(struct timespec *req, struct timespec *rem)
{
	int ret;

	ret = nanosleep(req, rem);
	if(!ret){	/* normal end */
		return -1;
	}
	else if(errno != EINTR){
		openlog("wd-keepalive", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_NOTICE, "%d: nanosleep() errno=%d\n", __LINE__, errno);
		closelog();
		return -2;
	}
	else{	/* ret == EINTR */
		openlog("wd-keepalive", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_NOTICE, "%d: nanosleep() receive EINTR", __LINE__);
		closelog();
		req->tv_sec = rem->tv_sec;
		req->tv_nsec = rem->tv_nsec;
	}
	return 0;
}

void usage(void)
{
	printf("watchdog daemon for Openblocks HX1\n\n");
	printf("obshx1-wdt [-i interval] [-t timeout]\n");
	printf("option:\n");
	printf("\t-i : interval second (default=30)\n");
	printf("\t-t : timeout second (default=60)\n");
}

int main(int ac, char* av[])
{
	char buf[100];
	int pid;
	int fd;
	int ret;
	int i;
	struct timespec req, rem;
	extern char *optarg;	

	while ((i = getopt(ac, av, "i:t:")) != -1){
		switch(i){
		case 'i':
			interval = strtoul(optarg, NULL, 10);
			break;
		case 't':
			timeout = strtoul(optarg, NULL, 10);
			break;
		default:
			usage();
			return -1;
		}
	}

	if(interval == 0 || timeout == 0){
		usage();
		return -1;
	}

	if((pid = fork())){
		/* parent */
		if((fd = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) < 0){
			exit(-1);
		}
		sprintf(buf, "%d\n", pid);
		if(write(fd, buf, strlen(buf)) != strlen(buf)){
			close(fd);
			exit(-1);
		}
		close(fd);
	}
	else{
#if !defined(DEBUG)
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		if(setsid() == -1){
			exit(-1);
		}

		signal(SIGTERM, die);
		AXBoardSetIndexValue(EAPI_AX_HWMON_WDT_ENABLE, 0, timeout);

		forever=1;
		while(forever){
			AXBoardSetIndexValue(EAPI_AX_HWMON_WDT_RELOAD, 0, 0);
			req.tv_sec = interval;
			req.tv_nsec = 0;
			for(;;) {
				if((ret = sleep_interval(&req, &rem)) == -1){
					break;
				}
				else if(ret == -2){
					forever = 0;
					break;
				}
			};
		}
		AXBoardSetIndexValue(EAPI_AX_HWMON_WDT_STOP, 0, 0);
		openlog("wd-keepalive", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: loop exit\n", __LINE__);
		closelog();
	}

	return 0;
}
