//#define DEBUG
/*	$ssdlinux: runled_bx1.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
/*
 * Copyright (c) 2008-2022 Plat'Home CO., LTD.
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
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <syslog.h>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <i2c/smbus.h>
#endif

#define PID_FILE "/var/run/segled.pid"

#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9)
#define SEGLED_DEV_R	"/sys/class/gpio/gpio342/value"
#define SEGLED_DEV_G	"/sys/class/gpio/gpio343/value"
#define SEGLED_DEV_B	"/sys/class/gpio/gpio344/value"
#elif defined(CONFIG_OBSA16)
#define SEGLED_DEV_R   "/sys/class/leds/red/brightness"
#define SEGLED_DEV_G   "/sys/class/leds/green/brightness"
#define SEGLED_DEV_B   "/sys/class/leds/blue/brightness"
#else
#define SEGLED_DEV_R	"/sys/class/gpio/gpio47/value"
#define SEGLED_DEV_G	"/sys/class/gpio/gpio48/value"
#define SEGLED_DEV_B	"/sys/class/gpio/gpio49/value"
#endif

#if defined(CONFIG_OBSHX1)
#define I2C_NAME "/dev/i2c-0"
#define SLAVE_NUM 0x70

#define LED_R 0x01
#define LED_G 0x02
#define LED_B 0x04

enum{
	INPUT = 0,
	OUTPUT,
	POLARITY,
	CONFIG
};
#endif

/*	config file format			*/
/*	line 1		light up (sec)	*/
/*	line 2		light out (sec)	*/
/*	line 3		color number	*/
#define CONFIG_FILE		"/tmp/.runled"
#define LED_BLACK	0
#define LED_RED		1
#define LED_GREEN	2
#define LED_YELLOW	3
#define LED_BLUE	4
#define LED_PURPLE	5
#define LED_AQUA	6
#define LED_WHITE	7
#define SPEED_DEFAULT (10 * 1000)

static int led_up         = SPEED_DEFAULT;
static int led_down       = SPEED_DEFAULT;
static int led_up_color   = LED_BLACK;
static int led_down_color = LED_BLACK;
static int forever = 1;

#if 0
void donothing(int i){}
void die(int i){forever=0;}
#endif

void read_config(void)
{
	FILE *fp;
	char str[256];

	if((fp = fopen(CONFIG_FILE, "r")) == NULL){
		led_up_color   = LED_BLACK;
		led_down_color = LED_BLACK;
		led_up         = SPEED_DEFAULT;
		led_down       = SPEED_DEFAULT;
		return;
	}

	if(fgets(str, sizeof(str)-1, fp) == NULL){
		goto ERROR;
	}
	led_up = (int)strtol(str, NULL, 10);
	if(!led_up || led_up == LONG_MAX || led_up == LONG_MIN)
		led_up = SPEED_DEFAULT;

	if(fgets(str, sizeof(str)-1, fp) == NULL){
		goto ERROR;
	}
	led_down = (int)strtol(str, NULL, 10);
	if(led_down == LONG_MAX || led_down == LONG_MIN)
		led_down = SPEED_DEFAULT;

	if(fgets(str, sizeof(str)-1, fp) == NULL){
		goto ERROR;
	}
	led_up_color = (int)strtol(str, NULL, 10);
	if(led_up_color < 0 || led_up_color > 7){
		goto ERROR;
	}
	if(led_up_color == LONG_MAX || led_up_color == LONG_MIN){
		goto ERROR;
	}

	if(fgets(str, sizeof(str)-1, fp) == NULL){
		//goto ERROR;
		// Upper Compatible

		led_down_color = LED_BLACK ;
		fclose(fp);
		return;
	}
	led_down_color = (int)strtol(str, NULL, 10);
	if(led_down_color < 0 || led_down_color > 7){
		goto ERROR;
	}
	if(led_down_color == LONG_MAX || led_down_color == LONG_MIN){
		goto ERROR;
	}

	fclose(fp);
	return;

ERROR:
	led_up_color = LED_BLACK;
	led_down_color = LED_BLACK;
	led_up = SPEED_DEFAULT;
	led_down = SPEED_DEFAULT;
	fclose(fp);
}

#if defined(CONFIG_OBSHX1)
int open_device(void)
{
	int fd;
	if((fd = open(I2C_NAME, O_RDWR)) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(ioctl(fd, I2C_SLAVE, SLAVE_NUM) < 0){
		close(fd);
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	return fd;
}
#endif

void set_color(char* b, char* g, char* r)
{
	int fd;

#if defined(CONFIG_OBSHX1)
	unsigned char led = 0;

	if(*r - '0') led |= LED_R;
	if(*g - '0') led |= LED_G;
	if(*b - '0') led |= LED_B;

	if((fd = open_device()) < 0)
		exit(-1);

	if(i2c_smbus_write_byte_data(fd, OUTPUT, led) == -1){
		close(fd);
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}

	close(fd);
#else
	if ((fd = open(SEGLED_DEV_R, O_RDWR)) < 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}
	write(fd, r, 1);
	close(fd);
	if ((fd = open(SEGLED_DEV_G, O_RDWR)) < 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}
	write(fd, g, 1);
	close(fd);
	if ((fd = open(SEGLED_DEV_B, O_RDWR)) < 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}
	write(fd, b, 1);

	close(fd);
#endif
}

void dancer(void)
{
	struct timespec req, rem;
	int i;

printf("%s %d\n", __func__, __LINE__);
	memset(&req, 0x0, sizeof(req));
	memset(&rem, 0x0, sizeof(rem));
	for (i=0;forever;i++){
		read_config();
		if(i % 2){
			switch(led_up_color){
			case LED_BLACK:
				set_color("0", "0", "0");
				break;
			case LED_RED:
				set_color("0", "0", "1");
				break;
			case LED_GREEN:
				set_color("0", "1", "0");
				break;
			case LED_YELLOW:
				set_color("0", "1", "1");
				break;
			case LED_BLUE:
				set_color("1", "0", "0");
				break;
			case LED_PURPLE:
				set_color("1", "0", "1");
				break;
			case LED_AQUA:
				set_color("1", "1", "0");
				break;
			case LED_WHITE:
				set_color("1", "1", "1");
				break;
			default:
				break;
			}

			req.tv_sec = led_up / 1000;
			req.tv_nsec = (led_up % 1000) * 1000 * 1000;
			while(nanosleep(&req, &rem) == -1)
				req.tv_nsec = rem.tv_nsec;
		}
		else{
			switch(led_down_color){
			case LED_BLACK:
				set_color("0", "0", "0");
				break;
			case LED_RED:
				set_color("0", "0", "1");
				break;
			case LED_GREEN:
				set_color("0", "1", "0");
				break;
			case LED_YELLOW:
				set_color("0", "1", "1");
				break;
			case LED_BLUE:
				set_color("1", "0", "0");
				break;
			case LED_PURPLE:
				set_color("1", "0", "1");
				break;
			case LED_AQUA:
				set_color("1", "1", "0");
				break;
			case LED_WHITE:
				set_color("1", "1", "1");
				break;
			default:
				break;
			}
			req.tv_sec = led_down / 1000;
			req.tv_nsec = (led_down % 1000) * 1000 * 1000;
			while(nanosleep(&req, &rem) == -1)
				req.tv_nsec = rem.tv_nsec;
		}
	}
printf("%s %d\n", __func__, __LINE__);
}

int
main(void)
{
	int fd;
	int pid;

	if (getuid()) {
		fprintf(stderr, "must be super user\n");
		return 1;
	}

	if ((pid = fork())) {
		/* parent */
		char tmp[100];
		if ((fd = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) == -1) {
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
printf("%s %d\n", __func__, __LINE__);
		if(setsid() == -1)
			exit(4);

#if 0
		/* child */
		signal( SIGHUP,donothing);
		signal( SIGINT,die);
		signal( SIGQUIT,die);
		signal( SIGILL,die);
		signal( SIGTRAP,die);
		signal( SIGABRT,die);
		signal( SIGIOT,die);
		signal( SIGBUS,die);
		signal( SIGFPE,die);
		signal( SIGKILL,die);
		signal( SIGUSR1,die);
		signal( SIGSEGV,die);
		signal( SIGUSR2,die);
		signal( SIGPIPE,die);
		signal( SIGALRM,die);
		signal( SIGTERM,die);
		signal( SIGSTKFLT,die);
		signal( SIGCHLD,die);
		signal( SIGCONT,die);
		signal( SIGSTOP,die);
		signal( SIGTSTP,die);
		signal( SIGTTIN,die);
		signal( SIGTTOU,die);
		signal( SIGURG,die);
		signal( SIGXCPU,die);
		signal( SIGXFSZ,die);
		signal( SIGVTALRM,die);
		signal( SIGPROF,die);
		signal( SIGWINCH,die);
		signal( SIGIO,die);
		signal( SIGPWR,die);
		signal( SIGSYS,die);
#endif
printf("%s %d\n", __func__, __LINE__);
		dancer();
printf("%s %d\n", __func__, __LINE__);
		exit(0);
	}
	return 0;
}
