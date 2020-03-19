//#define DEBUG 1
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
#include <unistd.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <i2c/smbus.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>

#define DEVNAME "/dev/i2c"
#define POLY 0x1021 
#define BSIZ 32
#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9)
#define CHANNEL 7
#else
#define CHANNEL 1
#endif
#if defined(CONFIG_OBSIX9)
const unsigned char slave = 0xae >> 1;
#else
const unsigned char slave = 0xa0 >> 1;
#endif

#define swap(x,y) do{ unsigned char z=y; y=x; x=z; } while(0)

union CRC16{
	unsigned short s;
	unsigned char c[2];
};
union MODEM{
	unsigned long l;
	unsigned char c[4];
};

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

int read_modem(int i2cnum, char* fname, unsigned char* data)
{
	FILE *fp;
	union CRC16 crc16;
	unsigned short s;

	crc16.c[0] = data[30];
	crc16.c[1] = data[31];
	if(crc16.s == 0xffff){
		printf("blank\n");
		return -1;
	}
	s = CRC16_CCITT(&data[16], 3, 0xffff);
	swap(crc16.c[0], crc16.c[1]);
	if(crc16.s != s){
		printf("%d: ERROR %04x%04x\n", __LINE__, s, crc16.s);
		return -1;
	}

	if(data[0] != 0xfe){
		printf("%d: invalid offset value\n", __LINE__);
		return -1;
	}
	if((fp = fopen(fname, "w")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	fprintf(fp, "%02x%02x%02x\n", data[16], data[17], data[18]);
	fclose(fp);

	return 0;
}

int write_modem(int i2cnum, char* fname, unsigned char* data)
{
#define SZ 6
	union CRC16 crc16;
	union MODEM modem;
	FILE *fp;
	char buf[SZ+1];
	char s[3];
	char head;
	int i;

	if((fp = fopen(fname, "r")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	memset(buf, 0x0, SZ+1);
	if(fread(buf, SZ, 1, fp) != 1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	fclose(fp);
	if(strlen(buf) != SZ){
		printf("%d: ERROR\n", __LINE__);
		return -1;
	}

	for(i=0; i<3; i++){
		s[0] = buf[i*2];
		s[1] = buf[i*2+1];
		s[2] = 0x0;
		modem.c[i] = strtol(s, NULL, 16);
	}

	/* write modem */
	for(i=0; i<3; i++){
		if(write_i2c(i2cnum, slave, i+16, modem.c[i]) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
	}

	if((modem.l & 0xffffff) != 0xffffff){
		/* CRC16 */
		crc16.s = CRC16_CCITT(modem.c, 3, 0xffff);
		swap(crc16.c[0], crc16.c[1]);
		for(i=0; i<2; i++){
			if(write_i2c(i2cnum, slave, i+30, crc16.c[i]) == -1){
				printf("%d: %s\n", __LINE__, strerror(errno));
				return -1;
			}
		}
		head = 0xfe;
	}
	else{
		crc16.c[0] = 0xff;
		crc16.c[1] = 0xff;
		for(i=0; i<2; i++){
			if(write_i2c(i2cnum, slave, i+30, crc16.c[i]) == -1){
				printf("%d: %s\n", __LINE__, strerror(errno));
				return -1;
			}
		}
		head = 0xff;
	}

	/* write headder */
	if(write_i2c(i2cnum, slave, 0, head) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	return 0;
#undef SZ
}

int read_serial(int i2cnum, char* fname, unsigned char* data)
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

int write_serial(int i2cnum, char* fname, unsigned char* data)
{
#define SZ 8
	char buf[SZ+1];
	char head = 0xff;
	int i;
	int fd;

	if((fd = open(fname, O_RDONLY)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	memset(buf, 0x0, SZ+1);
	if(read(fd, buf, SZ) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	if(strlen(buf) != SZ){
		printf("%d: ERROR\n", __LINE__);
		return -1;
	}

	data[1] = buf[0];
	if(buf[1] <= '9')
		data[2] = buf[1]-0x30;
	else
		data[2] = buf[1]-0x37;
	data[3] = buf[2];
	data[4] = buf[3]-0x30;
	data[5] = buf[4]-0x30;
	data[6] = buf[5]-0x30;
	data[7] = buf[6]-0x30;
	data[8] = buf[7]-0x30;
	for(i=1; i<9; i++){
		if(write_i2c(i2cnum, slave, i, data[i]) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
	}

	/* write headder */
	if(data[0] < 0xfe){
		if(write_i2c(i2cnum, slave, 0, head) == -1){
			printf("%d: %s\n", __LINE__, strerror(errno));
			return -1;
		}
	}

	return 0;
#undef SZ
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
//		write modem type		obs-util -wm7 filename
//
int main(int ac, char* av[])
{
	int i;
	char buf[2];
	unsigned char data[BSIZ];
	unsigned char c;
	int access=0;
	int modem=0;
	int i2cnum=CHANNEL;

	if(ac == 1)
		return 0;

	while((i=getopt(ac, av, "wm0123456789abcdef")) != -1){
		switch(i){
		case 'w':
			access = 1;
#ifdef DEBUG
printf("%s\n", access ? "write" : "read");
#endif
			break;
		case 'm':
			modem = 1;
#ifdef DEBUG
printf("%s\n", modem ? "modem" : "serial");
#endif
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
			buf[0] = i; buf[1] = 0;
			i2cnum = strtol(buf, NULL, 16);
#ifdef DEBUG
printf("%d\n", i2cnum);
#endif
			break;
		default:
			printf("ERR%d\n", __LINE__);
			exit(1);
			break;
		}
	}
	ac -= optind;
	av += optind;
	if(ac != 1){
		printf("ERR%d\n", __LINE__);
		exit(1);
	}

#ifdef DEBUG
printf("%s-%d %x\n", DEVNAME, i2cnum, slave);
#endif
	memset(data, 0x0, BSIZ);
	for(i=0; i<BSIZ; i++){
		if(read_i2c(i2cnum, slave, i, &c) == -1){
			return -1;
		}
		data[i] = c;
	}
#ifdef DEBUG
printf("ac=%d av0=%s\n", ac, av[0]);
if(data[1] == 0xf){
	printf("%c%X%c%d%d%d%d%d\n",
		data[1]+0x37, data[2], data[3]+0x37, data[4],
		data[5], data[6], data[7], data[8]);
}
else{
	printf("%c%X%c%d%d%d%d%d\n",
		data[1], data[2], data[3], data[4],
		data[5], data[6], data[7], data[8]);
}
printf("%02x %02x%02x %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %02x%02x\n",
data[16], data[17], data[18], data[19],
data[20], data[21], data[22], data[23],
data[24], data[25], data[26], data[27],
data[28], data[29], data[30], data[31]); 
#endif
	if(data[0] < 0xfe){
		printf("%d: invalid offset value\n", __LINE__);
		exit(1);
	}

	if(modem){
		if(access){
			if(write_modem(i2cnum, av[0], data) == -1)
				return -1;
		}
		else{
			if(read_modem(i2cnum, av[0], data) == -1)
				return -1;
		}
	}
	else{
		if(access){
			if(write_serial(i2cnum, av[0], data) == -1)
				return -1;
		}
		else{
			if(read_serial(i2cnum, av[0], data) == -1)
				return -1;
		}
	}

	return 0;
}
