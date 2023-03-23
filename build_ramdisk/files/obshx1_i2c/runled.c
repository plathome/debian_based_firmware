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
#define PCA9538_PIN	0x10

enum {
	PCA9538_INPUT = 0,	
	PCA9538_OUTPUT,
	PCA9538_POLARITY,
	PCA9538_CONFIG
};

/* config file format      */
/* line 1  light up (sec)  */
/* line 2  light out (sec) */
/* line 3  color number    */

#define CONFIG_FILE	"/tmp/.runled"
#define PID_FILE	"/var/run/segled.pid"

#define LED_R		0x01
#define LED_G		0x02
#define LED_B		0x04

#define LED_BLACK	0
#define LED_RED		1
#define LED_GREEN	2
#define LED_YELLOW	3
#define LED_BLUE	4
#define LED_PURPLE	5
#define LED_AQUA	6
#define LED_WHITE	7

#define SPEED_DEFAULT (10 * 1000)

uint8_t flag_debug;

struct conf_t {
	int led_up;
	int led_down;
	int led_up_color;
	int led_down_color;
	time_t mtime;
};

void usages(const char *prog)
{
	printf("\n");
	printf("usages: %s [-d][-c <conf file>]\n", prog);	
	printf("    -c conf file  Conf file. [default: %s]\n", CONFIG_FILE);
	printf("    -d            Debug mode, running foreground.");
	printf("\n");
}

void rd_conf(const char *fconf, struct conf_t *conf)
{
	struct stat sb;
	FILE *fp;	
	char buf[256];	
	size_t len;

	len = sizeof(buf) - 1;

	if (stat(fconf, &sb) < 0) {
		goto ERROR_0;
	}

	if (conf->mtime == sb.st_mtime) {
		return;
	}

	conf->mtime = sb.st_mtime;

	if ((fp = fopen(fconf, "r")) == NULL) {	
		goto ERROR_0;
	}

       	if (fgets(buf, len, fp) == NULL) {
                goto ERROR_1;
        }
        conf->led_up = (int)strtol(buf, NULL, 10);
        if (!conf->led_up || conf->led_up == LONG_MAX || conf->led_up == LONG_MIN) {
                conf->led_up = SPEED_DEFAULT;
	}

        if (fgets(buf, len, fp) == NULL) {
                goto ERROR_1;
        }
        conf->led_down = (int)strtol(buf, NULL, 10);
        if (conf->led_down == LONG_MAX || conf->led_down == LONG_MIN) {
                conf->led_down = SPEED_DEFAULT;
	}

        if (fgets(buf, len, fp) == NULL) {
                goto ERROR_1;
        }
        conf->led_up_color = (int)strtol(buf, NULL, 10);
        if (conf->led_up_color < 0 || conf->led_up_color > 7) {
                goto ERROR_1;
        }
        if (conf->led_up_color == LONG_MAX || conf->led_up_color == LONG_MIN) {
                goto ERROR_1;
        }

        if (fgets(buf, len, fp) == NULL) {
                conf->led_down_color = LED_BLACK ;
                fclose(fp);
                return;
        }
        conf->led_down_color = (int)strtol(buf, NULL, 10);
        if (conf->led_down_color < 0 || conf->led_down_color > 7) {
                goto ERROR_1;
        }
        if (conf->led_down_color == LONG_MAX || conf->led_down_color == LONG_MIN) {
                goto ERROR_1;
        }

	if (flag_debug) {
		printf("%s(): up=%d down=%d up_color=%d down_color=%d\n",
			__func__, conf->led_up, conf->led_down, conf->led_up_color, conf->led_down_color);
	}

        fclose(fp);
        return;


ERROR_1:
	fclose(fp);
ERROR_0:
	conf->led_up         = SPEED_DEFAULT;
	conf->led_down       = SPEED_DEFAULT;
	conf->led_up_color   = LED_BLACK;	
	conf->led_down_color = LED_BLACK;
	
	return;
}

void set_color(char* b, char* g, char* r)
{
	uint8_t	rc, led = 0;

	if (*r - '0') led |= LED_R;
	if (*g - '0') led |= LED_G;
	if (*b - '0') led |= LED_B;	

	led ^= 0xff;

	if ((rc = i2cBusWriteByteWord(PCA9538_ADDR, PCA9538_OUTPUT, &led, writeByte)) != STATUS_SUCCESS) {
		fprintf(stderr, "%s(): i2cBusWriteByteWord(OUTPUT) error : %u\n", __func__, rc);
	}

	if (flag_debug) {
		uint8_t buf;
		rc = i2cBusReadByteWord(PCA9538_ADDR, PCA9538_INPUT, &buf, readByte);
		printf("%s(): write=0x%02x read=0x%02x rc=%u\n", __func__, led, buf, rc);
	}
	

	return;
}

void dancer(const char *fconf)
{
	uint8_t rc, buf;
	int i, sw = 0;
	struct timespec req, rem;
	struct conf_t conf;

	memset(&req, 0x0, sizeof(struct timespec));	
	memset(&rem, 0x0, sizeof(struct timespec));	
	conf.mtime = (time_t)0;

	buf = PCA9538_PIN;
	if ((rc = i2cBusWriteByteWord(PCA9538_ADDR, PCA9538_POLARITY, &buf, writeByte)) != STATUS_SUCCESS) {
		fprintf(stderr, "%s(): i2cBusWriteByteWord(POLARITY) error : %u\n", __func__, rc);
	}
	if ((rc = i2cBusWriteByteWord(PCA9538_ADDR, PCA9538_CONFIG, &buf, writeByte)) != STATUS_SUCCESS) {
		fprintf(stderr, "%s(): i2cBusWriteByteWord(CONFIG) error : %u\n", __func__, rc);
	}

	for (i=0; 1; i++) {	
		rd_conf(fconf, &conf);
		if(i % 2) {
			switch(conf.led_up_color) {
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
			req.tv_sec = conf.led_up / 1000;
			req.tv_nsec = (conf.led_up % 1000) * 1000 * 1000;
			while(nanosleep(&req, &rem) == -1) {
				req.tv_nsec = rem.tv_nsec;
			}
		}
		else {
			switch(conf.led_down_color) {
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
			req.tv_sec = conf.led_down / 1000;
			req.tv_nsec = (conf.led_down % 1000) * 1000 * 1000;
			while(nanosleep(&req, &rem) == -1) {
				req.tv_nsec = rem.tv_nsec;
			}
		}

		if (i == LONG_MAX) {
			i = 0;
		}
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

	dancer(fconf);

	return 0;	
}
