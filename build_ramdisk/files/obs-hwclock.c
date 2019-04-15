/*	$ssdlinux: obs-hwclock.c,v 0.01 2014/01/07 07:19:59 yamagata Exp $	*/
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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include <errno.h>

# define CORRECT_VALUE	0x9a

#if defined(CONFIG_OBSVX1)
#define	I2C_DEV	"/dev/i2c-5"
#else
#define	I2C_DEV	"/dev/i2c-1"
#endif
#define	STATUS1		0x30
#define	STATUS2		0x31
#define	DATE		0x32
#define	TIME		0x33
#define	INT1		0x34
#define	INT2		0x35
#define	CORRECT		0x36
#define	FREE		0x37

#define OP_SYSTOHC		"--systohc"
#define OP_HCTOSYS		"--hctosys"
#define OP_CLEARINT1	"--clearint1"
#define OP_CLEARINT2	"--clearint2"
#define OP_ALARM1		"--alarm1"
#define OP_ALARM2		"--alarm2"
#define OP_INIT			"--init"
#define OP_STATUS		"--status"
#define OP_CHECK		"--check"
#define OP_CORRECT		"--correct"
#define OP_HELP			"--help"

unsigned char reverse_8bit(unsigned char x)
{
	x = ((x & 0x55) << 1) | ((x & 0xaa) >> 1);
	x = ((x & 0x33) << 2) | ((x & 0xcc) >> 2);

	return (x << 4) | (x >> 4);
}

int do_check(void)
{
	int fd;
	unsigned char c[1];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, STATUS1) == -1){
		close(fd);
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(read(fd, c, 1) == -1){
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int do_init(void)
{
	int fd;
	unsigned char c[1];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, STATUS1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	c[0] = 0xC0;	/* set RESET bit and 12/24 bit */
	if(write(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	sleep(1);

	if(read(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	if(c[0] & 0x1 && c[0] & 0x2){	/* check POC bit and BLD bit */
		printf("%d: Can't initialize, please execute once again\n", __LINE__);
		exit(-2);
	}

	return 0;
}

int chk_init(void)
{
	int fd;
	unsigned char c[1];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	/* read status1 register */
	if(ioctl(fd, I2C_SLAVE, STATUS1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(c[0] & 0x1){		/* check POC bit */
		sleep(1);		/* wait 500 ms */
		printf("detected POC bit, do initialize\n");
		do_init();
	}
	else{
		if(c[0] & 0x2){	/* check BLD bit */
			printf("detected BLD bit, do initialize\n");
			do_init();
		}
	}
	return 0;
}

char* do_rawtime(void)
{
	static char buf[128];

	struct tm rtc, *tm;
	time_t sec;
	int fd;
	int i;
	unsigned char c[7];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return NULL;
	}

	if(ioctl(fd, I2C_SLAVE, DATE) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return NULL;
	}
	if(read(fd, c, 7) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return NULL;
	}
	close(fd);

	for(i=0; i<7; i++){
		if(i==4)
			c[i] &= 0xfc;
		c[i] = reverse_8bit(c[i]);
		c[i] = (c[i] & 0x0f) + ((c[i]>>4) & 0x0f) * 10;
	}
	rtc.tm_year = c[0] + 100;
	rtc.tm_mon = c[1] - 1;
	rtc.tm_mday = c[2];
	rtc.tm_wday = c[3];
	rtc.tm_hour = c[4];
	rtc.tm_min = c[5];
	rtc.tm_sec = c[6];

	if((sec = mktime(&rtc)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return NULL;
	}
	tm = localtime(&sec);
	sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d\n",
		tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
		tm->tm_min, tm->tm_sec);

	return buf;
}


int do_systohc(char *str)
{
	struct tm *tm;
	time_t sec;
	unsigned char c[7], d[7];
	int i;
	int fd;

	sec = strtol(str, NULL, 10);
	if(!sec || sec == LONG_MAX || sec == LONG_MIN){
		printf("%d: %s(%s)\n", __LINE__, strerror(errno), str);
		return -1;
	}

	sec += timezone;
	tm = gmtime(&sec);
	c[0] = (tm->tm_year - 100);
	c[1] = tm->tm_mon + 1;
	c[2] = tm->tm_mday;
	c[3] = tm->tm_wday;
	c[4] = tm->tm_hour;
	c[5] = tm->tm_min;
	c[6] = tm->tm_sec + 1;
	memcpy(d, c, sizeof(d));

	for(i=0; i<7; i++){
		c[i] = reverse_8bit(((c[i] / 10)<<4) + (c[i] % 10));
	}

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, DATE) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, c, 7) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(read(fd, c, 7) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	for(i=0; i<7; i++){
		if(i==4){
			c[i] &= 0xfc;
		}
		c[i] = reverse_8bit(c[i]);
		c[i] = (c[i] & 0x0f) + ((c[i]>>4) & 0x0f) * 10;
		if(c[i] != d[i]){
			printf("%d: RTC write error!(i=%d, c=%x, d=%x\n",
											__LINE__, i, c[i], d[i]); 
			exit(-1);
		}
	}

	return 0;
}

int do_hctosys(void)
{
	struct tm rtc, *tm;
	time_t sec;
	int fd;
	int i;
	unsigned char c[7];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, DATE) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, c, 7) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	for(i=0; i<7; i++){
		if(i==4)
			c[i] &= 0xfc;
		c[i] = reverse_8bit(c[i]);
		c[i] = (c[i] & 0x0f) + ((c[i]>>4) & 0x0f) * 10;
	}
	rtc.tm_year = c[0] + 100;
	rtc.tm_mon = c[1] - 1;
	rtc.tm_mday = c[2];
	rtc.tm_wday = c[3];
	rtc.tm_hour = c[4];
	rtc.tm_min = c[5];
	rtc.tm_sec = c[6];

	if((sec = mktime(&rtc)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
	}
	sec -= timezone;
	tm = localtime(&sec);
	printf("%04d/%02d/%02d %02d:%02d:%02d\n",
		tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
		tm->tm_min, tm->tm_sec);

	return 0;
}

int do_alarm(char *str, int mode)
{
	const char *daystr[] = {
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat"
	};
	int fd;
	unsigned char c[7], d[7];
	char time[3], *endp;

	if(str == NULL){
		if((fd = open(I2C_DEV, O_RDWR)) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}

		if(ioctl(fd, I2C_SLAVE, mode) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
		if(read(fd, c, 3) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
		close(fd);

		d[0] = c[0] & 0x1;
		d[1] = c[1] & 0x1;
		d[2] = c[2] & 0x1;
		c[0] = reverse_8bit(c[0] & 0xfe);
		c[0] = (c[0] & 0x0f) + ((c[0]>>4) & 0x0f) * 10;
		c[1] = reverse_8bit(c[1] & 0xfc);
		c[1] = (c[1] & 0x0f) + ((c[1]>>4) & 0x0f) * 10;
		c[2] = reverse_8bit(c[2] & 0xfe);
		c[2] = (c[2] & 0x0f) + ((c[2]>>4) & 0x0f) * 10;

		if(d[0])
			printf("day:%s ", daystr[c[0]]);
		if(d[1])
			printf("hour:%02d ", c[1]);
		if(d[2])
			printf("minute:%02d\n", c[2]);

		return 0;
	}

	memset(c, 0x0, sizeof(c));
	switch(strlen(str)){
	case 5:	/* wHHMM */
		time[0] = str[0]; time[1] = 0;
		c[0] = (unsigned char)strtol(time, &endp, 10);
		c[0] = reverse_8bit(((c[0] / 10)<<4) + (c[0] % 10)) | 0x1;
		time[0] = str[1]; time[1] = str[2]; time[2] = 0;
		c[1] = (unsigned char)strtol(time, &endp, 10);
		if(c[1] < 12)
			c[1] = reverse_8bit(((c[1] / 10)<<4) + (c[1] % 10)) | 0x1;
		else
			c[1] = reverse_8bit(((c[1] / 10)<<4) + (c[1] % 10)) | 0x3;
		time[0] = str[3]; time[1] = str[4]; time[2] = 0;
		c[2] = (unsigned char)strtol(time, &endp, 10);
		c[2] = reverse_8bit(((c[2] / 10)<<4) + (c[2] % 10)) | 0x1;
		break;
	case 4:	/* HHMM */
		time[0] = str[0]; time[1] = str[1]; time[2] = 0;
		c[1] = (unsigned char)strtol(time, &endp, 10);
		if(c[1] < 12)
			c[1] = reverse_8bit(((c[1] / 10)<<4) + (c[1] % 10)) | 0x1;
		else
			c[1] = reverse_8bit(((c[1] / 10)<<4) + (c[1] % 10)) | 0x3;
		time[0] = str[2]; time[1] = str[3]; time[2] = 0;
		c[2] = (unsigned char)strtol(time, &endp, 10);
		c[2] = reverse_8bit(((c[2] / 10)<<4) + (c[2] % 10)) | 0x1;
		break;
	case 2:	/* MM */
		time[0] = str[0]; time[1] = str[1]; time[2] = 0;
		c[2] = (unsigned char)strtol(time, &endp, 10);
		c[2] = reverse_8bit(((c[2] / 10)<<4) + (c[2] % 10)) | 0x1;
		break;
	default:
		printf("%d: parameter error\n", __LINE__);
		return -1;
	}

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, STATUS2) == -1){
		close(fd);
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	c[6] = 0x2;
	if(write(fd, &c[6], 1) == -1){
		close(fd);
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, mode) == -1){
		close(fd);
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, c, 3) == -1){
		close(fd);
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	close(fd);

	return 0;
}

int do_clearint(int mask)
{
	int fd;
	unsigned char c[1];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, STATUS1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, STATUS2) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	c[0] &= ~mask;

	if(write(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	return 0;
}

int do_status(void)
{
	int fd;
	unsigned char c[3];

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	/* read status1 register */
	if(ioctl(fd, I2C_SLAVE, STATUS1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	/* read status2 register */
	if(ioctl(fd, I2C_SLAVE, STATUS2) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, &c[1], 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	/* read correct register */
	if(ioctl(fd, I2C_SLAVE, CORRECT) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(read(fd, &c[2], 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	printf("status1 = %x\n", c[0]);
	printf("status2 = %x\n", c[1]);
	printf("correct = %x\n", c[2]);
	printf("raw time = %s\n", do_rawtime());

	return 0;
}

#if 0
int do_correct(char *str)
{
	int fd;
	unsigned char c[1];
	float f;
	char *endp;

	if(str == NULL){
		if((fd = open(I2C_DEV, O_RDWR)) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}

		if(ioctl(fd, I2C_SLAVE, CORRECT) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
		if(read(fd, c, 1) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
		close(fd);

		if(!c[0]){				// 0
			printf("Correction value = 0\n");
		}
		else if(c[0] & 0x02){	// minus
			c[0] &= 0xfc;
			c[0] = ~c[0]; 
			f = (float)reverse_8bit(c[0] & 0xfc) * 3 * 3600 * 24 / 1000000 + 0.5;
			c[0] = (unsigned char)f;
			printf("Correction value = -%d\n", c[0]);
		}
		else{					// plus
			f = (float)reverse_8bit(c[0] & 0xfc) * 3 * 3600 * 24 / 1000000 + 0.5;
			c[0] = (unsigned char)f;
			printf("Correction value = %d\n", c[0]);
		}

		return 0;
	}

	f = (float)strtol(str, &endp, 10);
	if(!f){ 
		c[0] = 0;
	}
	else if(f < 0){
		f = f / 3600 / 24 * 1000000 * -1;
		c[0] = f / 3;
		c[0] = ~c[0];
		c[0] = reverse_8bit((c[0] & 0x3f) | 0x40);
	}
	else if(f > 0){
		f = f / 3600 / 24 * 1000000;
		c[0] = f / 3;
		c[0] = reverse_8bit(c[0] & 0x3f);
	}

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, CORRECT) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	return 0;
}
#else
int do_correct(void)
{
	int fd;
	unsigned char c[1] = {CORRECT_VALUE};	// -10.368 sec/day

	if((fd = open(I2C_DEV, O_RDWR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, CORRECT) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(write(fd, c, 1) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	close(fd);

	return 0;
}
#endif

int main(int ac, char* av[])
{
	int ret=0;

	if(ac == 1){
		return -1;
	}
	else if(strncmp(av[1], OP_SYSTOHC, strlen(OP_SYSTOHC)) == 0){
		if(ac != 3){
			return -1;
		}
		ret = do_systohc(av[2]);
	}
	else if(strncmp(av[1], OP_HCTOSYS, strlen(OP_HCTOSYS)) == 0){
		if(ac != 2){
			return -1;
		}
		ret = do_hctosys();
	}
	else if(strncmp(av[1], OP_ALARM1, strlen(OP_ALARM1)) == 0){
		if(ac !=2 && ac != 3){
			return -1;
		}
		ret = do_alarm(av[2], INT1);
	}
	else if(strncmp(av[1], OP_ALARM2, strlen(OP_ALARM2)) == 0){
		if(ac !=2 && ac != 3){
			return -1;
		}
		ret = do_alarm(av[2], INT2);
	}
	else if(strncmp(av[1], OP_CLEARINT1, strlen(OP_CLEARINT1)) == 0){
		if(ac != 2){
			return -1;
		}
		ret = do_clearint(0x20);
	}
	else if(strncmp(av[1], OP_CLEARINT2, strlen(OP_CLEARINT2)) == 0){
		if(ac != 2){
			return -1;
		}
		ret = do_clearint(0x2);
	}
	else if(strncmp(av[1], OP_INIT, strlen(OP_INIT)) == 0){
		if(ac != 2){
			return -1;
		}
		ret = do_init();
		if(!ret){
			ret = do_correct();
		}
	}
	else if(strncmp(av[1], OP_STATUS, strlen(OP_STATUS)) == 0){
		if(ac != 2){
			return -1;
		}
		ret = do_status();
	}
	else if(strncmp(av[1], OP_CHECK, strlen(OP_CHECK)) == 0){
		if(ac != 2){
			return -1;
		}
		ret = do_check();
	}
#if 0
	else if(strncmp(av[1], OP_CORRECT, strlen(OP_CORRECT)) == 0){
		if(ac !=2 && ac != 3){
			return -1;
		}
		ret = do_correct(av[2]);
	}
#endif
	else if(strncmp(av[1], OP_HELP, strlen(OP_HELP)) == 0){
		return -1;
	}
	else{
		return -1;
	}

	return ret;
}
