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

#ifndef _I2CBUS_H_
#define _I2CBUS_H_

#define I2C_STATUS	0xFA60
#define I2C_CONTROL	0xFA61
#define I2C_COMMAND	0xFA62
#define I2C_ADDR	0xFA63
#define I2C_DATA0	0xFA64
#define I2C_DATA1	0xFA65
#define I2C_ClearStatus	0x00
#define I2C_byteAccess	0x91
#define I2C_WordAccess	0xA1
#define I2C_BlockAccess	0xC1
#define I2C_BlockNeDa	0xFE
#define I2C_Timeout	(time_t)500000

#define STATUS_SUCCESS	0
#define STATUS_BUSY	1
#define STATUS_FAIL	2
#define STATUS_TIMEOUT	3

typedef enum _I2CBUS_OPERATION {
	readByte,
	writeByte,
	readWord,
	writeWord,
	readBlock,
	writeBlock
} I2CBUS_OPERATION;

uint8_t i2cBusReadByteWord(
	uint8_t addr,
	uint8_t command,
	uint8_t *buf,
	I2CBUS_OPERATION operation);

uint8_t i2cBusWriteByteWord(
	uint8_t addr,
	uint8_t command,
	uint8_t *buf,
	I2CBUS_OPERATION operation);
  
uint8_t i2cBusReadBlock(
	uint8_t addr,
	uint8_t command,
	uint8_t block_size,
	uint8_t *buf);
   
uint8_t i2cBusWriteBlock(
	uint8_t addr,
	uint8_t command,
	uint8_t block_size,
	uint8_t *buf);

uint8_t i2cBusExecute(
	uint8_t slave_addr, 
	uint8_t command,
	I2CBUS_OPERATION operation,
	uint8_t *len,
	uint8_t *buf);

int i2cBusInit(void);

#endif // _I2CBUS_H_
