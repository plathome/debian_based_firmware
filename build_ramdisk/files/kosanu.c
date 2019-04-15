/*	$ssdlinux: kosanu.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
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
#include <unistd.h>
#include <limits.h>
#include <linux/i2c-dev.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <i2c/smbus.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/*
 *	usage
 *		read  : kosanu r channel slave addr
 *		write : kosanu w channel slave addr data
 *
 *		channel 	i2c bus number 			( 1 - 8 )
 *		slave		slave address
 *		addr		read or write address	( 0x0 - 0x3f )
 *		data		write data				( 0x0 - 0xff )
 *
 *		return
 *			read  : read data
 *			write : write data
 *			error : 0x800000xx 	xx = errno
 *					0x90000000	parameter error
 */

#define DEVNAME "/dev/i2c"

int write_i2c(unsigned char, unsigned char, unsigned char, unsigned char);
int read_i2c(unsigned char, unsigned char, unsigned char);
long chk_param(char *);

int write_i2c(unsigned char i2cnum, unsigned char slave,
								unsigned char addr, unsigned char data)
{
	int fd;
#if !defined(CONFIG_OBSVX1)
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

#if defined(CONFIG_OBSVX1)
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

int read_i2c(unsigned char i2cnum, unsigned char slave, unsigned char addr)
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
		return 0x80000000 | errno;
	}
	if(ioctl(fd, I2C_SLAVE, slave) < 0) {
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}

#if defined(CONFIG_OBSVX1)
	if((c = i2c_smbus_read_byte_data(fd, addr)) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}
#else
	if(write(fd, &addr, 1) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}
	if(read(fd, &c, 1) < 0){
		printf("ERR%d\n", __LINE__);
		close(fd);
		return 0x80000000 | errno;
	}
#endif
	close(fd);

	return (int)c;
}

long chk_param(char *param){
	long ret;
	char *endp;

	/* check parameter */
	ret = strtol(param, &endp, 0);
#ifdef DEBUG
printf("ret=%lx\n", ret);
#endif
	if((ret == LONG_MIN || ret == LONG_MAX) && errno != 0){
		printf("ERR%d\n", __LINE__);
		return 0x80000000 | errno;
	}
	else if(endp == param){
		printf("ERR%d\n", __LINE__);
		return 0x90000000;
	}
	else if(ret > INT_MAX){
		printf("ERR%d\n", __LINE__);
		return 0x90000000;
	}
	else if(ret < INT_MIN){
		printf("ERR%d\n", __LINE__);
		return 0x90000000;
	}
	else if(*endp != '\0'){
		printf("ERR%d\n", __LINE__);
		return 0x90000000;
	}
	return ret;
}

int main(int ac, char* av[])
{
	long ret;
	unsigned char i2cnum, slave, addr, data;

	if(!(*av[1] == 'r' && ac == 5) && !(*av[1] == 'w' && ac == 6)){
		printf("ERR%d\n", __LINE__);
		return 0x90000000;
	}

	/* i2c channel */
	if((ret = chk_param(av[2])) >= 0){
		i2cnum = (unsigned char)ret;
	}
	else{
			printf("ERR%d\n", __LINE__);
			return ret;
	}
	/* i2c slave address */
	if((ret = chk_param(av[3])) >= 0){
		slave = (unsigned char)ret >> 1;
	}
	else{
			printf("ERR%d\n", __LINE__);
			return ret;
	}
	/* i2c address */
	if((ret = chk_param(av[4])) >= 0){
		addr = (unsigned char)ret;
		if(addr > 0x7f){
			printf("ERR%d\n", __LINE__);
			return 0x90000000;
		}
	}
	else{
			printf("ERR%d\n", __LINE__);
			return ret;
	}

	if(av[1][0] == 'w'){
		/* write data */
		if((ret = chk_param(av[5])) >= 0){
			if(ret < 0 || ret > 0xff){
				printf("ERR%d\n", __LINE__);
				return 0x90000000;
			}
			data = (unsigned char)ret;
		}
		else{
				printf("ERR%d\n", __LINE__);
				return ret;
		}
		return write_i2c(i2cnum, slave, addr, data);
	}
	else{
		return read_i2c(i2cnum, slave, addr);
	}
	return 0x90000000;
}
