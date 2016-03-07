/*	$ssdlinux: obs-util.c,v 1.17 2014/01/07 07:19:06 yamagata Exp $	*/
/*
 * Copyright (c) 2008-2016 Plat'Home CO., LTD.
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
#include <errno.h>

const char *path = "/usr/sbin/";
const char *slave = "0xa0";
const char *command = "kosanu";
const char *channel = "1";

int main(int ac, char* av[])
{
	FILE *fp;
	int i;
	char buf[512];
	unsigned char serial[9];

	if(ac != 2){
		printf("parameter error.\n");
		return 1;
	}

	memset(serial, 0x0, sizeof(serial));
	for(i=0; i<9; i++){
		sprintf(buf, "%s%s r %s %s 0x%x", path, command, channel, slave, i);
		serial[i] = system(buf) >> 8;
	}
	if(serial[0] != 0xff){
		printf("WARNING: offset 0x0 is not 0xff!\n");
	}

	if((fp = fopen(av[1], "w")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return 1;
	}

	if(serial[1] == 0xf){
		fprintf(fp, "%c%X%c%d%d%d%d%d\n",
			serial[1]+0x37, serial[2], serial[3]+0x37, serial[4],
			serial[5], serial[6], serial[7], serial[8]);
	}
	else{
		fprintf(fp, "%c%X%c%d%d%d%d%d\n",
			serial[1], serial[2], serial[3], serial[4],
			serial[5], serial[6], serial[7], serial[8]);
	}

	fclose(fp);

	return 0;
}
