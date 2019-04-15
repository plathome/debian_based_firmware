/*	$ssdlinux: obsvx1-modem.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <i2c/smbus.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
 *	usage obsvx1-modem
 *
 *	Init mode 
 *	obsvx1-modem init
 *
 *	Read mode 
 *	obsvx1-modem read
 *
 *	Write mode 
 *	obsvx1-modem write value
 *
 *	value	= 0x00-0xff
 */

#define I2C_NAME "/dev/i2c-5"
#define SLAVE 0x20
#define INIT_MODEM0	0x70
#define INIT_MODEM1	0xef

#define POWER "power"
#define RESET1 "reset1"
#define RESET2 "reset2"
#define USBRST "usbrst"
#define UARTINI "uartini"
#define AREAIND "areaind"
#define PSHOLD "pshhold"
#define RSTCHK "rstchk"
#define RI "ri"
//#define FUPSTS "fupsts"
#define ESIM "esim"
#define ANT0 "ant0"
#define ANT1 "ant1"
#define MOSIND "mosind"
#define INIT "init"
#define RAW "raw"

#define M_POWER		0x01
#define M_USBRST	0x02
#define M_RESET1	0x04
#define M_RESET2	0x08
#define M_UARTINI	0x80
#define M_AREAIND	0x01
#define M_PSHOLD	0x02
#define M_RSTCHK	0x04
#define M_RI		0x08
//#define M_FUPSTS	0x10
#define M_ESIM		0x10
#define M_ANT0		0x20
#define M_ANT1		0x40
#define M_MOSIND	0x80

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
	printf("reset1 [high|low]: change high/low reset of module1\n");
	printf("reset2 [high|low]: change high/low reset of module2\n");
	printf("usbrst [high|low]: change high/low reset of usb hub\n");
	printf("esim   [high|low]: change high/low enable of eSIM\n");
	printf("init             : initialize gpio\n");
	printf("raw              : read raw data (DEBUG)\n");
#if 0
	printf("\nfor KYM1x\n");
	printf("uartini          : initialize uart\n");
	printf("areaind          : area ind\n");
	printf("pshhold          : psh hold\n");
	printf("rstchk           : rst chk\n");
	printf("ri               : ri\n");
	printf("fupsts           : fupsts\n");
	printf("ant0             : antinf_0\n");
	printf("ant1             : antinf_1\n");
	printf("mosind           : mos_ind\n");
#endif
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

int read_gpio(int reg, unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if((*val = i2c_smbus_read_byte_data(fd, reg)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int write_gpio(int reg, unsigned char val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if(i2c_smbus_write_byte_data(fd, reg, val) == -1){
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

	if(i2c_smbus_write_byte_data(fd, CONFIG0, INIT_MODEM0) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if(i2c_smbus_write_byte_data(fd, CONFIG1, INIT_MODEM1) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[0] = i2c_smbus_read_byte_data(fd, CONFIG0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[1] = i2c_smbus_read_byte_data(fd, CONFIG1)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	printf("init modem: %02x%02x\n", val[1], val[0]);

	return 0;
}

int read_register(int reg, unsigned char mask)
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

int write_register(int reg, char *flag, unsigned char mask)
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

int read_config(unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if((val[0] = i2c_smbus_read_byte_data(fd, CONFIG0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[1] = i2c_smbus_read_byte_data(fd, CONFIG1)) == -1){
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

	if((fd = open_device()) < 0)
		return -1;

	if((val[0] = i2c_smbus_read_byte_data(fd, OUTPUT0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[1] = i2c_smbus_read_byte_data(fd, OUTPUT1)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
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
	else if(strncmp(POWER, av[1], strlen(POWER)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_POWER)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_POWER)) == -1)
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
			if((ret = read_register(INPUT0, M_RESET2)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_RESET2)) == -1)
				return -1;
		}
	}
	else if(strncmp(USBRST, av[1], strlen(USBRST)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT0, M_USBRST)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT0, av[2], M_USBRST)) == -1)
				return -1;
		}
	}
	else if(strncmp(ESIM, av[1], strlen(ESIM)) == 0){
		if(ac == 2){
			if((ret = read_register(INPUT1, M_ESIM)) == -1)
				return -1;
			printf("%s\n", ret ? "high" : "low");
		}
		else{
			if((ret = write_register(OUTPUT1, av[2], M_ESIM)) == -1)
				return -1;
		}
	}
#if 0
	else if(strncmp(UARTINI, av[1], strlen(UARTINI)) == 0){
	}
	else if(strncmp(AREAIND, av[1], strlen(AREAIND)) == 0){
	}
	else if(strncmp(PSHOLD, av[1], strlen(PSHOLD)) == 0){
	}
	else if(strncmp(RSTCHK, av[1], strlen(RSTCHK)) == 0){
	}
	else if(strncmp(RI, av[1], strlen(RI)) == 0){
	}
	else if(strncmp(FUPSTS, av[1], strlen(FUPSTS)) == 0){
	}
	else if(strncmp(ANT0, av[1], strlen(ANT0)) == 0){
	}
	else if(strncmp(ANT1, av[1], strlen(ANT1)) == 0){
	}
	else if(strncmp(MOSIND, av[1], strlen(MOSIND)) == 0){
	}
#endif
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

