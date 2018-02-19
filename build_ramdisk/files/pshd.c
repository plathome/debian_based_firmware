//#define DEBUG
/*	$ssdlinux: obs600_pshd.c,v 1.12 2014/01/07 07:19:59 yamagata Exp $	*/
/*
 * Copyright (c) 2009-2018 Plat'Home CO., LTD.
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
/*
 * Push SW deamon
 */

#include <sys/types.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "obspushsw.h"
#include <linux/version.h>
#include <time.h>
#include <sys/time.h>

#define PID_FILE "/var/run/pshd.pid"

void donothing(int i);
void die(int i);

#ifdef DEBUG
#define PSW_DEBUG(str...)       printf ("pshd: " str)
#else
#define PSW_DEBUG(str...)       /* nothing */
#endif

#define MIN_SEC		1
#define MAX_SEC		3600

#define INTERVAL	200 * 1000	// 200ms(5 times a second)
#define SEGLED_PID	"/var/run/segled.pid"
#define SEGLED_DEV	"/dev/segled"

#ifdef CONFIG_LINUX_3_11_X
#define PUSHSW_DEV	"/dev/input/event0"
#if defined(CONFIG_OBSA7)
#define SEGLED_DEV_G	"/sys/class/leds/obsa7:green:stat/brightness"
#define SEGLED_DEV_Y	"/sys/class/leds/obsa7:yellow:stat/brightness"
#define SEGLED_DEV_R	"/sys/class/leds/obsa7:red:stat/brightness"
#elif defined(CONFIG_OBSA6)
#define SEGLED_DEV_G	"/sys/class/leds/obsa6:green:stat/brightness"
#define SEGLED_DEV_Y	"/sys/class/leds/obsa6:yellow:stat/brightness"
#define SEGLED_DEV_R	"/sys/class/leds/obsa6:red:stat/brightness"
#else
#define SEGLED_DEV_G	"/sys/class/leds/green_led/brightness"
#define SEGLED_DEV_Y	"/sys/class/leds/yellow_led/brightness"
#define SEGLED_DEV_R	"/sys/class/leds/red_led/brightness"
#endif
#elif defined(CONFIG_LINUX_4_0) && !defined(CONFIG_OBS600)
#define PUSHSW_DEV		"/dev/input/event0"
#if defined(CONFIG_OBSA7)
#define SEGLED_DEV_G	"/sys/devices/platform/gpio-leds/leds/obsa7:green:stat/brightness"
#define SEGLED_DEV_Y	"/sys/devices/platform/gpio-leds/leds/obsa7:yellow:stat/brightness"
#define SEGLED_DEV_R	"/sys/devices/platform/gpio-leds/leds/obsa7:red:stat/brightness"
#elif defined(CONFIG_OBSA6)
#define SEGLED_DEV_G	"/sys/devices/platform/gpio-leds/leds/obsa6:green:stat/brightness"
#define SEGLED_DEV_Y	"/sys/devices/platform/gpio-leds/leds/obsa6:yellow:stat/brightness"
#define SEGLED_DEV_R	"/sys/devices/platform/gpio-leds/leds/obsa6:red:stat/brightness"
#else
#define SEGLED_DEV_G	"/sys/devices/platform/soc/soc:internal-regs/soc:internal-regs:leds/leds/green_led/brightness"
#define SEGLED_DEV_Y	"/sys/devices/platform/soc/soc:internal-regs/soc:internal-regs:leds/leds/yellow_led/brightness"
#define SEGLED_DEV_R	"/sys/devices/platform/soc/soc:internal-regs/soc:internal-regs:leds/leds/red_led/brightness"
#endif
#else
#define PUSHSW_DEV	"/dev/pushsw"
#endif

static int forever	= 1;		// exit() flag
static int reboot	= 2 * 5;	// reboot time(default = 2 sec)
static int halt		= 5 * 5;	// shutdown time(default = 5 sec)

/* some variables used in getopt (3) */
extern char *optarg;

void usage(void)
{
	fprintf(stderr, "pshd [-r sec] [-h sec]\n");
	fprintf(stderr, "-r sec\tsecond to reboot\n");
	fprintf(stderr, "-h sec\tsecond to shutdown\n");
	fprintf(stderr, "%d <= sec <= %d & reboot < halt\n", MIN_SEC, MAX_SEC);
	fprintf(stderr, "\n");
	fprintf(stderr, "* Orange LED (Upper) turns on when time up for reboot\n");
	fprintf(stderr, "* Green LED (Middle) turns on in the middle of reboot and halt\n");
	fprintf(stderr, "* Red LED (Lower) turns on when time up for halt\n");
}

static inline void flash_led(int fd, char* num)
{
#if (defined(CONFIG_LINUX_3_11_X) || defined(CONFIG_LINUX_4_0)) && !defined(CONFIG_OBS600)
	int fh;
	switch(num[0]){	
	case '1':
		if ((fh = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "0", 1);
		close(fh);
		if ((fh = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "0", 1);
		close(fh);
		if ((fh = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "1", 1);
		close(fh);
		break;
	case '2':
		if ((fh = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "0", 1);
		close(fh);
		if ((fh = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "0", 1);
		close(fh);
		if ((fh = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "1", 1);
		close(fh);
		break;
	case '4':
		if ((fh = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "0", 1);
		close(fh);
		if ((fh = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "0", 1);
		close(fh);
		if ((fh = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		write(fh, "1", 1);
		close(fh);
		break;
	default:
		break;
	}
#else
	write(fd, num, 1);
#endif
}

#if (defined(CONFIG_LINUX_3_11_X) || defined(CONFIG_LINUX_4_0)) && !defined(CONFIG_OBS600)
static int get_push_event(int fd)
{
	const unsigned char pushsw_ev[] = {	/* pushsw event id */
		0x01, 0x00, 0x74, 0x00
	};
	static int flag = 0;
	int ret;
	unsigned char buf[16];

	if((ret = read(fd, buf, sizeof(buf))) == sizeof(buf)){
		/* pushsw event ? */
		if(memcmp(&buf[8], pushsw_ev, sizeof(pushsw_ev)) == 0){
			/* pushsw on ? */
			if(buf[12] == 0x1)
				flag = 1;
			else
				flag = 0;
			return flag;
		}
	}
	else if(ret == -1){
		/* EAGAIN == no event */
		if(errno != EAGAIN){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
	}
	return flag;
}
#endif

void watch_pushsw(void)
{
	FILE *fp;
	int fd, ledfd=0, rv;
	int count = 0;		// time of push INIT switch
	char buf[16];
	struct timespec req, rem;

	if ((fd = open(PUSHSW_DEV, O_RDONLY | O_NONBLOCK)) < 0) {
		exit(-1);
	}

	while(forever){
		memset(&rem, 0x0, sizeof(rem));
		req.tv_sec = 0;
		req.tv_nsec = INTERVAL * 1000;

#if (defined(CONFIG_LINUX_3_11_X) || defined(CONFIG_LINUX_4_0)) && !defined(CONFIG_OBS600)
		rv = get_push_event(fd);
#else
		rv = ioctl(fd, PSWIOC_GETSTATUS, NULL);
#endif
		if(rv == 0 && count){		/* INIT switch release */
			if(count >= reboot && count < halt){
				PSW_DEBUG("reboot ON\n");
				execl("/sbin/shutdown", "shutdown", "-r", "now", NULL);
			}
			else if(count >= halt){
				PSW_DEBUG("halt ON\n");
				execl("/sbin/shutdown", "shutdown", "-h", "now", NULL);
			}

			if(count){
				if(ledfd > 0)
					close(ledfd);
			}
			count=0;
		}
		else if(rv > 0){		/* INIT switch pushed */
			count++;
			if(count >= reboot){
				/* kill runled daemon */
				if((fp = fopen(SEGLED_PID, "r")) != NULL){
					if(fgets(buf, sizeof(buf)-1, fp) != NULL){
						PSW_DEBUG("runled pid=%d\n", atoi(buf));
						kill(atoi(buf), SIGTERM);
					}
					fclose(fp);
				}
#if (defined(CONFIG_LINUX_3_11_X) || defined(CONFIG_LINUX_4_0)) && !defined(CONFIG_OBS600)
				ledfd = 100;	/* dummy fd */
#else
				if((ledfd = open(SEGLED_DEV, O_RDWR)) == -1)
					ledfd = 0;
#endif
			}
			if(count >= reboot && count < halt){
				if(ledfd > 0)
#if defined(HAVE_PUSHSW_OBS600_H) || defined(CONFIG_OBS600)
					flash_led(ledfd, "1");
#else
					flash_led(ledfd, "4");
#endif
				if(count >= ((halt - reboot) / 2)){
					if(ledfd > 0)
						flash_led(ledfd, "2");
				}
			}
			else if(count >= halt){
				if(ledfd > 0)
#if defined(HAVE_PUSHSW_OBS600_H) || defined(CONFIG_OBS600)
					flash_led(ledfd, "4");
#else
					flash_led(ledfd, "1");
#endif
			}
		}
		else if(rv == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		while(nanosleep(&req, &rem) == -1)
			req.tv_nsec = rem.tv_nsec;
	}
	close(fd);
}

int main(int argc, char *argv[])
{
	int fd;
	int pid;
	int i;

	if (getuid()) {
		fprintf(stderr, "must run super user\n");
		return 1;
	}

	while ((i = getopt(argc, argv, "r:h:t")) != -1) {
		switch (i) {
		case 'r':
			reboot = atoi(optarg) < MAX_SEC ? atoi(optarg) * 5 : MAX_SEC * 5;
			if(!reboot) reboot++;	// 0 is no use
			break;
		case 'h':
			halt = atoi(optarg) < MAX_SEC ? atoi(optarg) * 5 : MAX_SEC * 5;
			if(!halt) halt++;	// 0 is no use
			break;
		default:
			usage();
			return (0);
		}
	}

	if (halt < reboot) {
		fprintf(stderr, "Please set it so that halt bigger than reboot.\n");
		return(1);
	}

	if ((halt - reboot) < (2 * 5)) {
		// for Green LED
		fprintf(stderr, "Please add differences more than two seconds to reboot and halt.\n");
		return(1);
	}

	if ((pid = fork())) {
		/* parent */
		char tmp[100];
		if ((fd = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		sprintf(tmp, "%d\n", pid);
		if (write(fd, tmp, strlen(tmp)) != strlen(tmp)) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			close(fd);
			exit(-2);
		}
		close(fd);
		return 0;
	} else {
#if !defined(DEBUG)
		/* daemon */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		if(setsid() == -1) {
			exit(4);
		}

		/* child */
		signal(SIGHUP,donothing);
		signal(SIGINT,die);
		signal(SIGQUIT,die);
		signal(SIGILL,die);
		signal(SIGTRAP,die);
		signal(SIGABRT,die);
		signal(SIGIOT,die);
		signal(SIGBUS,die);
		signal(SIGFPE,die);
		signal(SIGUSR1,die);
		signal(SIGSEGV,die);
		signal(SIGUSR2,die);
		signal(SIGPIPE,die);
		signal(SIGALRM,die);
		signal(SIGTERM,die);
		signal(SIGCHLD,die);
		signal(SIGCONT,die);
		signal(SIGSTOP,die);
		signal(SIGTSTP,die);
		signal(SIGTTIN,die);
		signal(SIGTTOU,die);
		signal(SIGURG,die);
		signal(SIGXCPU,die);
		signal(SIGXFSZ,die);
		signal(SIGVTALRM,die);
		signal(SIGPROF,die);
		signal(SIGWINCH,die);
		signal(SIGIO,die);
		signal(SIGPWR,die);
		signal(SIGSYS,die);

		watch_pushsw();

		exit(0);
	}
}

void donothing(int i) {
}
void die(int i) {
	forever = 0;
}

