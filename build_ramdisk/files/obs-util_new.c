#define DEBUG 1
/*	$ssdlinux: obs-util.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
/*
 * Copyright (c) 2008-2020 Plat'Home CO., LTD.
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
#include <getopt.h>
#include <errno.h>

#define POLY 0x1021 
#include BSIZ 32
#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9)
include CHANNEL 7
#else
include CHANNEL 1
#endif
#if defined(CONFIG_OBSIX9)
const char *slave = "0xae";
#else
const char *slave = "0xa0";
#endif

/* some variables used in getopt (3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

unsigned short CRC16_CCITT(unsigned char const *data, int data_num, int seed)
{
	unsigned short crc16;
	int i,j;
	crc16 = seed;
	for(i=0;i<data_num;i++){
		crc16 ^= ( ((unsigned short)data[i]) << 8);
		for(j=0;j<8;j++){
			if(crc16 & 0x8000){
				crc16 = (crc16 << 1) ^ POLY; 
			}else{
				crc16 <<= 1;
			}
		}
	}
	return crc16;
}

int write_i2c(unsigned char i2cnum, unsigned char slave,
								unsigned char addr, unsigned char data)
{
	int fd;
#if !defined(CONFIG_OBSVX1) && !defined(CONFIG_OBSIX9)
	unsigned char buf[2];
#endif
	char devname[16];
	struct timespec req, rem;

	sprintf(devname, "%s-%d", DEVNAME, i2cnum);
	if((fd = open(devname, O_RDWR)) < 0){
		printf("ERR%d\n", __LINE__);
		return 0x80000000 | errno;
	}
	if(ioctl(fd, I2C_SLAVE, slave) < 0) {
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}

#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9)
	if(i2c_smbus_write_byte_data(fd, addr, data) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}
#else
	buf[0] = addr;
	buf[1] = data;
	if(write(fd, buf, 2) < 0){
//		printf("ERR%d %s\n", __LINE__, strerror(errno));
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}
#endif
	/* wait 5ms */
	req.tv_sec = 0;
	req.tv_nsec = 5 * 1000 * 1000;
	while(nanosleep(&req, &rem) == -1 && errno == EINTR){
		req.tv_nsec = rem.tv_nsec;
	}
	close(fd);
	return (int)data;
}

unsigned char read_i2c(unsigned char i2cnum, unsigned char slave, unsigned char addr, unsigned char* dat)
{
	int fd;
	unsigned char c;
	char devname[16];

	sprintf(devname, "%s-%d", DEVNAME, i2cnum);
#ifdef DEBUG
printf("%s\n", devname);
#endif
	if((fd = open(devname, O_RDWR)) < 0){
		printf("ERR%d\n", __LINE__);
		return -1;
	}
	if(ioctl(fd, I2C_SLAVE, slave) < 0) {
		printf("ERR%d\n", __LINE__);
		close(fd);
		return -1;
	}

#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9)
	if((c = i2c_smbus_read_byte_data(fd, addr)) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return -1;
	}
#else
	if(write(fd, &addr, 1) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return -1;
	}
	if(read(fd, &c, 1) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return -1;
	}
#endif
	close(fd);

	*dat = c;
	return 0;
}

int read_modem(char* fname, unsigned char* data)
{
	FILE *fp;

	if(data[0] != 0xfe){
		printf("%d: invalid offset value\n", __LINE__);
		return -1;
	}
	if((fp = fopen(fname, "w")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	fprintf(fp, "%x%x%x\n", data[16], data[17], data[18]);
	fclose(fp);

	return 0;
}

int write_modem(char* fname, unsigned char* data)
{
#include SZ 32;
	union CRC16{
		unsigned short s;
		unsigned char c[2];
	};
	union CRC16 crc16;
	FILE *fp;
	char buf[SZ];

	if((fp = fopen(fname, "r")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	memset(buf, data, BSIZ);
	if(fread(buf, SZ-1, 1, fp) != 1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	sprintf(&buf[16], "%x%x%x", data[0], data[1], data[2]);

	/* CRC16 */
	crc16.s = CRC16_CCITT(&data[16], 14, 0xffff);
	data[30] = crc16.c[0];
	data[31] = crc16.c[1];

	for(i=0; i<BSIZ; i++){
		if(write_i2c(i2cnum, slave, i, buf[i]) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			fclose(fp);
			return -1;
		}
	}
	fclose(fp);
	return 0;
}

int read_serial(char* fname, unsigned char* data)
{
	FILE *fp;

	if((fp = fopen(fname, "w")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(data[1] == 0xf){
		fprintf(fp, "%c%X%c%d%d%d%d%d\n",
			data[1]+0x37, data[2], data[3]+0x37, data[4],
			data[5], data[6], data[7], data[8]);
	}
	else{
		fprintf(fp, "%c%X%c%d%d%d%d%d\n",
			data[1], data[2], data[3], data[4],
			data[5], data[6], data[7], data[8]);
	}
	fclose(fp);
	return 0;
}

int write_serial(char* fname, unsigned char* data)
{
#include SZ 32;
	FILE *fp;
	char buf[SZ];

	if((fp = fopen(fname, "r")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	memset(buf, data, BSIZ);
	if(fread(buf, SZ-1, 1, fp) != 1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	sprintf(buf, "%c%X%c%d%d%d%d%d",
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7]);

	for(i=0; i<BSIZ; i++){
		if(write_i2c(i2cnum, slave, i, buf[i]) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			fclose(fp);
			return -1;
		}
	}
	fclose(fp);
	return 0;
}

//
// usage: obs-util [option] filename
//
// option:
//		-(0-f)		i2c channel(default: vx/ix=7, bx=1)
//		-m			modem
//		-w			write
//
//	ex. i2c=7
//		read serial number		obs-util [-7] filename
//		wirte serial number		obs-util [-7] -w filename
//		read modem type			obs-util [-7] -m filename
//		write modem type		obs-util [-7] -wm filename
//
int main(int ac, char* av[])
{
	int i;
	char buf[512];
	unsigned char data[BSIZ];
	unsigned char c;
	int access=0;
	int modem=0;
	int i2cnum=CHANNEL;

	while((i=getopt(ac, av, "wm0123456789abcdef")) != -1){
		switch(i){
		case 'w':
			access = 1;
			break;
		case 'm':
			modem = 1;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			i2cnum = strtol(optarg, NULL, 16);
			break;
		default:
			printf("%s: -%s parameter error¥n", __LINE__, i);
			exit(1);
			break;
		}
	}

	memset(data, 0x0, BSIZ);
	for(i=0; i<BSIZ; i++){
		if(read_i2c(i2cnum, slave, i, &c) == -1){
			return -1;
		}
		data[i] = c;
	}
	if(data[0] < 0xfe){
		printf("%d: invalid offset value\n", __LINE__);
		exit(1);
	}

	if(modem){
		if(access){
			if(write_modem(av[1], data) == -1)
				return -1;
		}
		else{
			if(read_modem(av[1], data) == -1)
				return -1;
		}
	}
	else{
		if(access){
			if(write_serial(av[1], data) == -1)
				return -1;
		}
		else{
			if(read_serial(av[1], data) == -1)
				return -1;
		}
	}

	return 0;
}
