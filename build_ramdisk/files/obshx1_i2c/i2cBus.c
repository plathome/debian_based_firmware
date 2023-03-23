/*
 * Copyright (c) 2023 Plat'Home CO., LTD.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Plat'Home CO., LTD. nor the names of
 *	its countributors may be used to endorse or promote products derived
 *	from this software without specific prior written permission.
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
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/io.h>
#include <unistd.h>
#include "i2cBus.h"
							   
uint8_t i2cBusReadByteWord(
	uint8_t addr,
	uint8_t command,
	uint8_t *buf,
	I2CBUS_OPERATION operation)
{
	int count = 0;
	struct timeval tv;
	double timeout;

	//Check I2C Busy
	while (count < 10) {
		if ((inb(I2C_STATUS) & 0x02) != 0x02) {
	   		break;
		}
		usleep(30000);
		count++;
	}
	if (count == 10) {
		return STATUS_BUSY;
	}

	outb(I2C_ClearStatus, I2C_STATUS);
	outb(command, I2C_COMMAND);
	outb(addr + 1, I2C_ADDR);

	if (operation == readByte) {
		outb(I2C_byteAccess, I2C_CONTROL);
	}
	else if (operation == readWord) {
	   outb(I2C_WordAccess, I2C_CONTROL);
	}
	
	//Command Error
	gettimeofday(&tv, NULL);
	timeout = (double)tv.tv_sec + (double)(tv.tv_usec + I2C_Timeout) / 1000000.0;
	do {
		if ((inb(I2C_STATUS) & 0x04) == 0x04) {
			return STATUS_FAIL;
		}
		gettimeofday(&tv, NULL);
		if ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0 > timeout) {
			return STATUS_TIMEOUT;
		} 
	} while (inb(I2C_STATUS)  != 0x08);
	
	*buf = (uint8_t)inb(I2C_DATA0);
	if (operation == readWord) {
	   	*(buf + 1) = inb(I2C_DATA1);
	}
	   
	outb(0x00, I2C_STATUS);

	return STATUS_SUCCESS;
}							   

uint8_t i2cBusWriteByteWord(
	uint8_t addr,
	uint8_t command,
	uint8_t *buf,
	I2CBUS_OPERATION operation)
{
	int count = 0;
	struct timeval tv;
	double timeout;

	//Check I2C Busy
	while (count < 10) {
		if ((inb(I2C_STATUS) & 0x02) != 0x02) {
	   		break;
		}
		usleep(30000);
		count++;
	}
	if (count == 10) {
		return STATUS_BUSY;
	}

	outb(I2C_ClearStatus, I2C_STATUS);
	outb(command, I2C_COMMAND);
	outb(addr, I2C_ADDR);
	outb(*buf, I2C_DATA0);

	if (operation == writeByte) {
		outb(I2C_byteAccess, I2C_CONTROL);
	}
	else if(operation == writeWord) {
		outb(*(buf + 1), I2C_DATA1);
		outb(I2C_WordAccess, I2C_CONTROL);
	}
	
	//Command Error
	gettimeofday(&tv, NULL);
	timeout = (double)tv.tv_sec + (double)(tv.tv_usec + I2C_Timeout) / 1000000.0;
	do {
		if ((inb(I2C_STATUS) & 0x04) == 0x04) {
			return STATUS_FAIL;
		}
		gettimeofday(&tv, NULL);
		if ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0 > timeout) {
			return STATUS_TIMEOUT;
		} 
	} while (inb(I2C_STATUS)  != 0x08);
	
	outb(0x00, I2C_STATUS);

	return STATUS_SUCCESS;
}

uint8_t i2cBusReadBlock(
	uint8_t addr,
	uint8_t command,
	uint8_t block_size,
	uint8_t *buf)
{
	int count = 0;
	int i;
	struct timeval tv;
	double timeout;

	//Check I2C Busy
	while (count < 10) {
		if ((inb(I2C_STATUS) & 0x02) != 0x02) {
	   		break;
		}
		usleep(30000);
		count++;
	}
	if (count == 10) {
		return STATUS_BUSY;
	}

	outb(I2C_ClearStatus, I2C_STATUS);
	outb(command, I2C_COMMAND);
	outb(addr + 1, I2C_ADDR);
	outb(block_size, I2C_DATA0);
	outb(I2C_BlockAccess, I2C_CONTROL);
	
	count = block_size - 1;
	 
	for (i=0 ; i < block_size ; i++) {
		while (1) {
			if (inb(I2C_DATA0) == count) {
				break;
			}
	  	}
	  	*(buf + i) = inb(I2C_DATA1);
	  	if (i < block_size - 1) {
			outb(I2C_BlockNeDa, I2C_STATUS );
		}
	  	usleep(200);

	  	count--;
	}

	//Command Error
	gettimeofday(&tv, NULL);
	timeout = (double)tv.tv_sec + (double)(tv.tv_usec + I2C_Timeout) / 1000000.0;
	do {
		if ((inb(I2C_STATUS) & 0x04) == 0x04) {
			return STATUS_FAIL;
		}
		gettimeofday(&tv, NULL);
		if ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0 > timeout) {
			return STATUS_TIMEOUT;
		} 
	} while ((inb(I2C_STATUS) & 0x08)  != 0x08);
	
	*(buf + i) = inb(I2C_DATA1);
	outb(0x00, I2C_STATUS);
	
	return STATUS_SUCCESS;
}

uint8_t i2cBusWriteBlock(
	uint8_t addr,
	uint8_t command,
	uint8_t block_size,
	uint8_t *buf)
{
	int i;
	int count = 0;
	struct timeval tv;
	double timeout;

	//Check I2C Busy
	while (count < 10) {
		if ((inb(I2C_STATUS) & 0x02) != 0x02) {
	   		break;
		}
		usleep(30000);
		count++;
	}
	if (count == 10) {
		return STATUS_BUSY;
	}

	outb(I2C_ClearStatus, I2C_STATUS);
	outb(command, I2C_COMMAND);
	outb(addr, I2C_ADDR);
	outb(block_size, I2C_DATA0);
	outb( *buf, I2C_DATA1);
	outb(I2C_BlockAccess, I2C_CONTROL);
	
	//Wait I2C Free
	count = 0;
	while (count < 10) {
		if ((inb(I2C_STATUS) & 0x02) != 0x02) {
	   		break;
		}
		usleep(30000);
		count++;
	}
	
	count = block_size-1;
	 
	for (i=1; i < block_size ; i++) {
	  	while (1) {
			if(inb( I2C_DATA0) == count) {
				break;
			}
	  	}
		outb(*(buf + i), I2C_DATA1);
	  	outb(I2C_BlockNeDa, I2C_STATUS );
	  	usleep(200);

	  	count--;
	}

	//Command Error
	gettimeofday(&tv, NULL);
	timeout = (double)tv.tv_sec + (double)(tv.tv_usec + I2C_Timeout) / 1000000.0;
	do {
		if ((inb(I2C_STATUS) & 0x04) == 0x04) {
			return STATUS_FAIL;
		}
		gettimeofday(&tv, NULL);
		if ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0 > timeout) {
			return STATUS_TIMEOUT;
		} 
	} while ((inb(I2C_STATUS) & 0x08)  != 0x08);

	outb(0x00, I2C_STATUS);
	
	return STATUS_SUCCESS;
}

uint8_t i2cBusExecute(
	uint8_t slave_addr, 
	uint8_t command,
	I2CBUS_OPERATION operation,
	uint8_t *len,
	uint8_t *buf)
{
	uint8_t rc;

	switch (operation) {
		case readByte:
		case readWord:
			rc = i2cBusReadByteWord(slave_addr, command, buf, operation);
			break;
	  	case writeByte:
		case writeWord:
			rc = i2cBusWriteByteWord(slave_addr, command, buf, operation);
			break;
		case readBlock:
			rc = i2cBusReadBlock(slave_addr, command, *len, buf);
			break;
		case writeBlock:
			rc = i2cBusWriteBlock(slave_addr, command, *len, buf);
			break;
		default:
			rc = STATUS_FAIL;
			break;
	}

	return rc;
}

int i2cBusInit(void)
{
	int rc;

	if ((rc = ioperm(I2C_STATUS, 8, 1)) < 0) {
		fprintf(stderr, "%s(): ioperm() error for 0x%x : %s\n", __func__, I2C_STATUS, strerror(errno));
	}

	return rc;
}
