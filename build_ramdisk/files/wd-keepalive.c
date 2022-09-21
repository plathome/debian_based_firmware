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
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#define PID_FILE	"/var/run/wd-keepalive.pid"
#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9) || defined(CONFIG_OBSA16) || defined(CONFIG_OBSHX1)
#define INTERVAL	20
#define WD_FILE		"/dev/watchdog0"
#else
#define INTERVAL	30
#define WD_FILE		"/dev/watchdog"
#endif

static int forever;

void die(int sig)
{
	forever=0;
}

int main(void)
{
	char buf[100];
	int pid;
	int fd, fd2;
	int ret;
	int opt=WDIOS_ENABLECARD;
	struct timespec req, rem;

	if((pid = fork())){
		/* parent */
		if((fd2 = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) < 0){
			exit(-1);
		}
		sprintf(buf, "%d\n", pid);
		if(write(fd2, buf, strlen(buf)) != strlen(buf)){
			close(fd2);
			exit(-1);
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
			exit(-1);
		}

		signal(SIGTERM, die);
		if((fd = open(WD_FILE, O_WRONLY)) == -1){
			exit(1);
		}
		if(ioctl(fd, WDIOC_SETOPTIONS, &opt) == -1){
			close(fd);
			exit(2);
		}

		forever=1;
		while(forever){
			if(ioctl(fd, WDIOC_KEEPALIVE, 0) == -1){
				close(fd);
				exit(3);
			}
			req.tv_sec = INTERVAL;
			req.tv_nsec = 0;
			for(;forever;){
				ret = nanosleep(&req, &rem);
				if(!ret){	/* normal end */
					break;
				}
				else if(errno != EINTR){
					openlog("wd-keepalive", LOG_CONS|LOG_PID, LOG_USER);
					syslog(LOG_NOTICE, "%d: nanosleep() errno=%d\n", __LINE__, errno);
					closelog();
					forever=0;
					break;
				}
				else{	/* ret == EINTR */
					openlog("wd-keepalive", LOG_CONS|LOG_PID, LOG_USER);
					syslog(LOG_NOTICE, "%d: nanosleep() receive EINTR", __LINE__);
					closelog();
					req.tv_sec = rem.tv_sec;
					req.tv_nsec = rem.tv_nsec;
				}
			}
		}
		opt = WDIOS_DISABLECARD;
		if(ioctl(fd, WDIOC_SETOPTIONS, &opt) == -1){
			close(fd);
			exit(4);
		}
		close(fd);
		openlog("wd-keepalive", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: loop exit\n", __LINE__);
		closelog();
	}

	return 0;
}
