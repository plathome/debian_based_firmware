//#define DEBUG
/*	$ssdlinux: obsiot-power.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
/*
 * Copyright (c) 2008-2018 Plat'Home CO., LTD.
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
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <i2c/smbus.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <linux/version.h>
#include <syslog.h>

extern int errno;

#define PID_FILE "/var/run/obsiot-power.pid"
#if defined(CONFIG_OBSVX1)
#define I2C_NAME	"/dev/i2c-2"
#elif defined(CONFIG_OBSBX1)
#define I2C_NAME	"/dev/i2c-1"
#elif defined(CONFIG_OBSGEM1)
#define I2C_NAME	"/dev/i2c-1"
#endif
#define SLAVE		0x21
#define INIT_BAT	0xfd
enum{
	STOP	= 0,
	START	= 2,
};

#if defined(CONFIG_OBSVX1)
#define POWER_USB	"/sys/class/gpio/gpio366/value"
#define POWER_AC	"/sys/class/gpio/gpio367/value"
#define POWER_DC	"/sys/class/gpio/gpio365/value"
#elif defined(CONFIG_OBSBX1)
#define POWER_USB	"/sys/class/gpio/gpio40/value"
#define POWER_AC	"/sys/class/gpio/gpio41/value"
#define POWER_DC	"/sys/class/gpio/gpio42/value"
#elif defined(CONFIG_OBSGEM1)
#define OFF_USB 113
#define OFF_AC 110
#define OFF_DC 114
#define POWER_USB	"/sys/class/gpio/gpio%d/value"
#define POWER_AC	"/sys/class/gpio/gpio%d/value"
#define POWER_DC	"/sys/class/gpio/gpio%d/value"
#endif

int chg_charging(unsigned char);

void donothing(int i){}
void die(int i){
	chg_charging(STOP);
	openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
	syslog(LOG_ERR, "%d: Stop battery charging\n", __LINE__);
	closelog();
	exit(0);
}

/* some variables used in getopt (3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;


int INTERVAL		= 10;
int LIMIT		= 300;
#if defined(CONFIG_OBSVX1)
#define DEF_COMMAND	"/sbin/poweroff"
#elif defined(CONFIG_OBSBX1)
#define DEF_COMMAND	"/sbin/halt"
#elif defined(CONFIG_OBSGEM1)
#define DEF_COMMAND	"/sbin/halt"
#endif
char COMMAND[1024];

enum{
	INPUT=0,
	OUTPUT,
	POLARITY,
	CONFIG,
};

int open_i2c(void)
{
	int fd;

	if((fd = open(I2C_NAME, O_RDWR)) < 0){
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
	if(ioctl(fd, I2C_SLAVE, SLAVE) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
	return fd;
}

int read_gpio(int reg, unsigned char *val)
{
	int fd;

	if((fd = open_i2c()) < 0)
		return -1;

#if defined(CONFIG_OBSVX1)
	if((*val = i2c_smbus_read_byte_data(fd, reg)) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	if(write(fd, &reg, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
	if(read(fd, val, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
	close(fd);

	return 0;
}

int write_gpio(int reg, unsigned char val)
{
	int fd;
#if ! defined(CONFIG_OBSVX1)
	unsigned char buf[2];
#endif

	if((fd = open_i2c()) < 0)
		return -1;

#if defined(CONFIG_OBSVX1)
	if(i2c_smbus_write_byte_data(fd, reg, val) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	buf[0] = reg;
	buf[1] = val;
	if(write(fd, buf, 2) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
	close(fd);

	return 0;
}

int chk_voltage(void)
{
	int fd;
	unsigned char val;
#if ! defined(CONFIG_OBSVX1)
	unsigned char reg = INPUT;
#endif

	if((fd = open_i2c()) < 0)
		return -1;

#if defined(CONFIG_OBSVX1)
	if((val = i2c_smbus_read_byte_data(fd, INPUT)) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	if(write(fd, &reg, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
	if(read(fd, &val, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
	close(fd);

	return (int)(val & 0x01);
}

int chg_charging(unsigned char val)
{
	int fd;
#if ! defined(CONFIG_OBSVX1)
	unsigned char buf[2];
#endif

	if((fd = open_i2c()) < 0)
		return -1;

#if defined(CONFIG_OBSVX1)
	if(i2c_smbus_write_byte_data(fd, OUTPUT, val) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	buf[0] = OUTPUT;
	buf[1] = val;
	if(write(fd, buf, 2) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
	close(fd);

	return 0;
}

int init_gpio(void)
{
	int fd;
	unsigned char val = 0;
#if ! defined(CONFIG_OBSVX1)
	unsigned char buf[2];
#endif

	if((fd = open_i2c()) < 0)
		return -1;

#if defined(CONFIG_OBSVX1)
	if(i2c_smbus_write_byte_data(fd, CONFIG, INIT_BAT) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	buf[0] = CONFIG;
	buf[1] = INIT_BAT;
	if(write(fd, buf, 2) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
#if defined(CONFIG_OBSVX1)
	if((val = i2c_smbus_read_byte_data(fd, CONFIG)) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	if(write(fd, buf, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
	if(read(fd, &val, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
	close(fd);

	printf("init config: %02x\n", val);
	return 0;
}

int get_power_status(void)
{
	int fd;
	int val;
#if ! defined(CONFIG_OBSVX1)
	unsigned char reg = INPUT;
#endif

	if((fd = open_i2c()) < 0)
		return -1;

#if defined(CONFIG_OBSVX1)
	if((val = i2c_smbus_read_byte_data(fd, INPUT)) == -1){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#else
	if(write(fd, &reg, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
	if(read(fd, &val, 1) < 0){
		close(fd);
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: %s\n", __LINE__, strerror(errno));
		closelog();
		return -1;
	}
#endif
	close(fd);

	if(val & 0x01){
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: BAT-L was detected, shutdown the machine.\n", __LINE__);
		closelog();
	}

	return 0;
}

int get_power_input(void)
{
	int fd;
	char val = 0;
	int usb=0, ac=0, dc=0;

	/* usb power */
	if ((fd = open(POWER_USB, O_RDONLY)) != -1){
		read(fd, &val, 1);
		close(fd);
		usb = val - '0';
	}
	else{
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: gpio usb open error\n", __LINE__);
		closelog();
	}

	/* AC power */
	if ((fd = open(POWER_AC, O_RDONLY)) != -1){
		read(fd, &val, 1);
		close(fd);
		ac = val - '0';
	}
	else{
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: gpio AC open error\n", __LINE__);
		closelog();
	}

	/* DC power */
	if ((fd = open(POWER_DC, O_RDONLY)) != -1){
		read(fd, &val, 1);
		close(fd);
		dc = val - '0';
	}
	else{
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: gpio DC open error\n", __LINE__);
		closelog();
	}

#if defined(DEBUG)
	printf("usb=%d ac=%d dc=%d\n", usb, ac, dc);
#endif
	return usb + ac + dc;
}

int chk_power()
{
	int count = -1;
	int ret;

	if(init_gpio() == -1){
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: Initialize error\n", __LINE__);
		closelog();
		return -1;
	}
	if(chg_charging(START) == -1){
		openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
		syslog(LOG_ERR, "%d: Error battery charging\n", __LINE__);
		closelog();
		return -1;
	}
	openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
	syslog(LOG_ERR, "%d: Start battery charging\n", __LINE__);
	closelog();

	for (;;) {
		if(!get_power_input()){
			if(count == -1){
				count = LIMIT / INTERVAL;	/* power lost */
				if(count < 0) count = 0;
				openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
				syslog(LOG_ERR, "%d: Power lost(count=%d)\n", __LINE__, count);
				closelog();

				chg_charging(STOP);
				openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
				syslog(LOG_ERR, "%d: Stop battery charging\n", __LINE__);
				closelog();
			}
		}
		else{
			if(count != -1){
				openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
				syslog(LOG_ERR, "%d: Power return\n", __LINE__);
				closelog();

				chg_charging(START);
			}
			count = -1;					/* power return */
		}

		if(!count){		/* timeup power lost */
			ret = system(COMMAND);
			openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
			syslog(LOG_ERR, "%d: system() retrun %d\n", __LINE__, ret);
			closelog();
			count = -1;
		}
		else if(count > 0){	/* continue power lost */
			count--;
//			openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
//			syslog(LOG_ERR, "%d: Continue power lost(count=%d)\n", __LINE__, count);
//			closelog();
		}

		if(count > 0 && !chk_voltage()){
			openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
			syslog(LOG_ERR, "%d: Detect low voltage\n", __LINE__);
			closelog();
			ret = system(COMMAND);
			openlog("obsiot-power", LOG_CONS|LOG_PID, LOG_USER);
			syslog(LOG_ERR, "%d: system() retrun %d\n", __LINE__, ret);
			closelog();
		}
		sleep(INTERVAL);
	}
}

void usage(char *fname)
{
	printf("usage : %s -i seconds -t seconds -C command\n", fname);
	printf("\n");
//	printf("\t-i : Monitaring interval(in seconds)\n");
	printf("\t-t : Execute command after specified time(in seconds)\n");
	printf("\t-C : Command to execute(strings)\n");
}

int
main(int ac, char *av[])
{
	int fd;
	int pid;
	int i;

	if (getuid()) {
		fprintf(stderr, "must be super user\n");
		return 1;
	}

	strcpy(COMMAND, DEF_COMMAND);
	while ((i = getopt(ac, av, "i:t:C:h")) != -1) {
		switch (i) {
		case 'i':
//			INTERVAL = strtol(optarg, NULL, 10);
			break;
		case 't':
			LIMIT = strtol(optarg, NULL, 10);
			break;
		case 'C':
			strncpy(COMMAND, optarg, sizeof(COMMAND)-1);
			break;
		case 'h':
			usage(av[0]);
			return (0);
		default:
			usage(av[0]);
			return (0);
		}
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
		/* daemon */
#if !defined(DEBUG)
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		if(setsid() == -1)
			exit(4);

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
		if(chk_power() == -1)
			return -1;
	}
	return 0;
}
