/*	$ssdlinux: obsvx1-modem.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/version.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
 *	usage obsfx1-modem
 *
 *	Init mode 
 *	obsfx1-modem init
 *
 *	Read mode 
 *	obsfx1-modem read
 *
 *	Write mode 
 *	obsfx1-modem write value
 *
 *	value	= 0x00-0xff
 */

#define I2C_NAME "/dev/i2c-1"
#define SLAVE 0x20
#define INIT_MODEM0	0x10
#define INIT_MODEM1	0xf8
#define INIT_OUTPUT0	0x2f
#define INIT_OUTPUT1	0x07

#define WLPWR "wlpwr"
#define WLDIS "wldis"
#define BTDIS "btdis"
#define S1PWR "s1pwr"
#define POWER "power"
#define RESET1 "reset1"
#define S1DIS "s1dis"
#define S1CTL "s1ctl"
#define S2PON  "s2pon"
#define RESET2 "reset2"
#define LOST "lost"
#define POE	 "poe"
#define USBRST "usbrst"

#define INIT "init"
#define RAW "raw"

#define M_WLPWR		0x01
#define M_WLDIS		0x02
#define M_BTDIS		0x04
#define M_S1PWR		0x08
#define M_POWER		0x10
#define M_RESET1	0x20
#define M_S1DIS		0x40
#define M_S1CTL		0x80
#define M_S2PON		0x01
#define M_RESET2	0x02
#define M_LOST		0x04
#define M_POE		0x08
#define M_USBRST	0x10

enum {
	INPUT0 = 0,
	INPUT1,
	OUTPUT0,
	OUTPUT1,
	POLARITY0,
	POLARITY1,
	CONFIG0,
	CONFIG1,
};

void usage(char *fname)
{
	printf("%s option\n", fname);
	printf("option :\n");
	printf("power  [high|low]: change high/low power of module1\n");
	printf("s1pwr  [high|low]: change high/low control PWR of module1\n");
	printf("reset1 [high|low]: change high/low reset of module1\n");
	printf("reset2 [high|low]: change high/low reset of module2\n");
	printf("usbrst [high|low]: change high/low reset of usb hub\n");
	printf("lost             : power lost detect signal\n");
	printf("poe              : poe power supply good signal\n");
	printf("init             : initialize gpio\n");
	printf("raw              : read raw data (DEBUG)\n");
	printf("wlpwr            : change high/low WiFi/BT power(DEBUG)\n");
	printf("wldis            : change high/low disalbe WiFi(DEBUG)\n");
	printf("btdis            : change high/low disalbe Bluetooth(DEBUG)\n");
}

int open_device(void)
{
	int fd;

	if((fd = open(I2C_NAME, O_RDWR)) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(ioctl(fd, I2C_SLAVE, SLAVE) < 0){
		close(fd);
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	return fd;
}

int read_gpio(unsigned char reg, unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if(write(fd, &reg, 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(read(fd, val, 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int write_gpio(unsigned char reg, unsigned char val)
{
	int fd;
	unsigned char buf[2];

	if((fd = open_device()) < 0)
		return -1;

	buf[0] = reg; buf[1] = val;
	if(write(fd, buf, 2) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int read_config(unsigned char *val)
{
	int fd;
	unsigned char buf[1];

	if((fd = open_device()) < 0)
		return -1;

	buf[0] = CONFIG0;
	if(write(fd, buf, 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(read(fd, &val[0], 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	buf[0] = CONFIG1;
	if(write(fd, buf, 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(read(fd, &val[1], 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int read_output(unsigned char *val)
{
	int fd;
	unsigned char buf[1];

	if((fd = open_device()) < 0)
		return -1;

	buf[0] = OUTPUT0;
	if(write(fd, buf, 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(read(fd, &val[0], 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	buf[0] = OUTPUT1;
	if(write(fd, buf, 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(read(fd, &val[1], 1) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

unsigned short init_gpio(void)
{
	int fd;
	unsigned char val[2] = {0, 0};

	if((fd = open_device()) < 0)
		return -1;

	/* CONFIG */
	val[0] = CONFIG0; val[1] = INIT_MODEM0;
	if(write(fd, val, 2) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	val[0] = CONFIG1; val[1] = INIT_MODEM1;
	if(write(fd, val, 2) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(read_config(val) < 0)
		return -1;
	printf("init modem: %02x%02x\n", val[1], val[0]);

	/* OUTPUT */
	val[0] = OUTPUT0; val[1] = INIT_OUTPUT0;
	if(write(fd, val, 2) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	val[0] = OUTPUT1; val[1] = INIT_OUTPUT1;
	if(write(fd, val, 2) < 0){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	if(read_output(val) < 0)
		return -1;
	printf("     output: %02x%02x\n", val[1], val[0]);

	return 0;
}

int read_register(unsigned char reg, unsigned char mask)
{
	unsigned char val=0;

	if(read_gpio(reg, &val) < 0){
		return -1;
	}
	if(val & mask)
		return 1;
	else
		return 0;
}

int write_register(unsigned char reg, char *flag, unsigned char mask)
{
	unsigned char val=0;

	if(read_gpio(reg, &val) < 0)
		return -1;

	if(strcmp("high", flag) == 0)
		val |= mask;
	else if(strcmp("low", flag) == 0)
		val &= ~mask;
	else
		return -1;

	if(write_gpio(reg, val) < 0)
		return -1;

	return read_register(reg, mask);
}

int main(int ac, char *av[])
{
	int ret=0;

	if(ac != 2 && ac != 3){
		usage(av[0]);
		return -1;
	}
	if(strncmp(INIT, av[1], strlen(INIT)) == 0){
		if(init_gpio() == -1)
			return -1;
	}
	else if(strncmp(WLPWR, av[1], strlen(WLPWR)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_WLPWR)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_WLPWR)) == -1)
				return -1;
		}
	}
	else if(strncmp(WLDIS, av[1], strlen(WLDIS)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_WLDIS)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_WLDIS)) == -1)
				return -1;
		}
	}
	else if(strncmp(BTDIS, av[1], strlen(BTDIS)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_BTDIS)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_BTDIS)) == -1)
				return -1;
		}
	}
	else if(strncmp(S1PWR, av[1], strlen(S1PWR)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_S1PWR)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_S1PWR)) == -1)
				return -1;
		}
	}
	else if(strncmp(POWER, av[1], strlen(POWER)) == 0){
		if(ac == 2){
//			if((ret = read_register(INPUT0, M_POWER)) == -1)
			if((ret = read_register(INPUT0, M_S1CTL)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
//			if((ret = write_register(OUTPUT0, av[2], M_POWER)) == -1)
			if((ret = write_register(OUTPUT0, av[2], M_S1CTL)) == -1)
				return -1;
		}
	}
	else if(strncmp(S1DIS, av[1], strlen(S1DIS)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_S1DIS)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_S1DIS)) == -1)
				return -1;
		}
	}
	else if(strncmp(RESET1, av[1], strlen(RESET1)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_RESET1)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_RESET1)) == -1)
				return -1;
		}
	}
	else if(strncmp(RESET2, av[1], strlen(RESET2)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT1, M_RESET2)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT1, av[2], M_RESET2)) == -1)
				return -1;
		}
	}
	else if(strncmp(USBRST, av[1], strlen(USBRST)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT1, M_USBRST)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT1, av[2], M_USBRST)) == -1)
				return -1;
		}
	}
	else if(strncmp(LOST, av[1], strlen(LOST)) == 0){
		if((ret = read_register(INPUT1, M_LOST)) == -1)
			return -1;
		printf("%s\n", ret ? "high" : "low");
	}
	else if(strncmp(POE, av[1], strlen(POE)) == 0){
		if((ret = read_register(INPUT1, M_POE)) == -1)
			return -1;
		printf("%s\n", ret ? "high" : "low");
	}
	else if(strncmp(RAW, av[1], strlen(RAW)) == 0){
		unsigned char val[2] = {0, 0};

		if(read_gpio(INPUT0, &val[0]) < 0)
			return -1;
		if(read_gpio(INPUT1, &val[1]) < 0)
			return -1;
		printf("input: %02x %02x\n", val[1], val[0]);
		if(read_config(val) < 0)
			return -1;
		printf("config: %02x %02x\n", val[1], val[0]);
		if(read_output(val) < 0)
			return -1;
		printf("output: %02x %02x\n", val[1], val[0]);
	}
	else{
		usage(av[0]);
		return -1;
	}

	return ret;
}

