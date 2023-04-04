/*
 * Copyright (c) 2023 Plat'Home CO., LTD.
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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "i2cBus.h"

#define	PCA9538_ADDR	0xE0
#define	PCA9538_PIN	0x10

enum {
	PCA9538_INPUT = 0,	
	PCA9538_OUTPUT,
	PCA9538_POLARITY,
	PCA9538_CONFIG
};

#define CONFIG_FILE	"/tmp/.pshd"
#define PID_FILE	"/var/run/pshd.pid"

uint8_t flag_debug;

void usages(const char *prog)
{
	printf("\n");
	printf("usages: %s [-d][-c <conf file>]\n", prog);	
	printf("    -c conf file  Conf file. [default: %s]\n", CONFIG_FILE);
	printf("    -d            Debug mode, running foreground.");
	printf("\n");
}

void system_exec(const char *fconf)
{
	FILE *fp;
	char buf[1024] ;	

	memset(buf , 0x00 , sizeof(buf));	

	if ((fp = fopen(fconf, "r"))  == NULL) {
		if (flag_debug) {
			printf("%s(): can not open %s\n", __func__, fconf);
		}
		return;
	}

	if ( fgets(buf , sizeof(buf) , fp) == NULL ) {	
		if (flag_debug) {
			printf("%s(): can not read %s\n", __func__, fconf);
		}
		fclose(fp);
		return;
	}

	fclose(fp);

	system(buf);	

	return;
}

void wtach_pushsw(const char *fconf)
{
	uint8_t rc; 
	char buf;

	buf = PCA9538_PIN;
	if ((rc = i2cBusWriteByteWord(PCA9538_ADDR, PCA9538_POLARITY, &buf, writeByte)) != STATUS_SUCCESS) {
		fprintf(stderr, "%s(): i2cBusWriteByteWord(POLARITY) error : %u\n", __func__, rc);
	}
	if ((rc = i2cBusWriteByteWord(PCA9538_ADDR, PCA9538_CONFIG, &buf, writeByte)) != STATUS_SUCCESS) {
		fprintf(stderr, "%s(): i2cBusWriteByteWord(CONFIG) error : %u\n", __func__, rc);
	}
	
	buf = 0;
	if (flag_debug) {
		rc = i2cBusReadByteWord(PCA9538_ADDR, PCA9538_INPUT, &buf, readByte);
		printf("%s(): register data 0x%02x rc=%u\n", __func__, buf & 0xff, rc);
		int i, j, k;
		for (i=0; i<16; i++) {
			for (j=0; j<16; j++) {
				k = i * 16 + j;
				rc = i2cBusReadByteWord(k, PCA9538_INPUT, &buf, readByte);
				printf(" 0x%02x:%u:0x%02x ", k, rc, buf & 0xff);

			}
			printf("\n");
		}
	}

	while (1) {
		buf = 0;
		rc = i2cBusReadByteWord(PCA9538_ADDR, PCA9538_INPUT, &buf, readByte);
		if (rc == STATUS_SUCCESS) {
			if (buf & PCA9538_PIN) {
				if (flag_debug) {
					printf("%s(): switch pushed\n", __func__, fconf);
				}
				system_exec(fconf);
				usleep(3000000);
			}
		}
		usleep(100000);
	}
}

int main(int argc, char *argv[])
{
	int i;
	const char *prog, *fconf;
	FILE *fp;

	extern char *optarg;	
	
	prog = (const char *)basename(argv[0]);
	fconf = (const char *)CONFIG_FILE;
	flag_debug = 0;

	while ((i = getopt(argc, argv, "c:dh?")) > 0) {
		switch (i) {
			case 'c':
				fconf = (const char *)optarg;
				break;
			case 'd':
				flag_debug = 1;
				break;
			case 'h':
			case '?':
			default:
				usages(prog);
				break;
		}
	}

	if (!flag_debug) {
		if (daemon(0,0) != 0) {
			fprintf(stderr, "%s(): daemon() : %s", __func__, strerror(errno));
			exit(-1);
		}
		if ((fp = fopen(PID_FILE, "w")) == 0) {
			fprintf(stderr, "%s(): can not open %s. : %s", __func__, PID_FILE, strerror(errno));
			exit(-2);
		}
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	i2cBusInit();

	wtach_pushsw(fconf);

	return 0;	
}
