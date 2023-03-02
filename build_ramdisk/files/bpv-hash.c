/*
 * Copyright (c) 2018 - 2023 Plat'Home CO., LTD.
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
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/aes.h>

#define DEFAULT_STORAGE "/dev/sda1"
#define DEFAULT_ENVFILE "./test.env"
/////// echo "obsconfig" | md5sum
#define DEFAULT_OFILE   "c0e0664261825532841696a661604925.env"

#define AOFFSET		1
#define WRSIZE		1024*32
#define AESSIZE		1024*32*4

//#define LDEBUG

const unsigned char     key   [16]  = { 0x4f, 0x70, 0x65, 0x6e, 0x42, 0x6c, 0x6f, 0x63, 0x6b, 0x73, 0x20, 0x20, 0x42, 0x50, 0x56, 0x34 };
const unsigned char     iv    [16]  = { 0x50, 0x6c, 0x61, 0x74, 0x27, 0x48, 0x6f, 0x6d, 0x65, 0x20, 0x4d, 0x6f, 0x6e, 0x74, 0x65, 0x72 };

int Base64Encode(char* message, int mes_size, char** buffer) {
	BIO *bio, *b64;
	FILE* stream;
	int encodedSize = 4*ceil((double)mes_size/3);
	*buffer = (char *)malloc(encodedSize+1);

	stream = fmemopen(*buffer, encodedSize+1, "w");
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(stream, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, message, mes_size);
	BIO_flush(bio);
	BIO_free_all(bio);
	fclose(stream);

	return (0); //success
}

int calcDecodeLength(char* b64input) {
	int len = strlen(b64input);
	int padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=')
		padding = 2;
	else if (b64input[len-1] == '=') //last char is =
		padding = 1;

	return (int)len*0.75 - padding;
}

int Base64Decode(char* b64message, int *length, char** buffer) {
	BIO *bio, *b64;
	int decodeLen = calcDecodeLength(b64message), len = 0;
	*buffer = (char*)malloc(decodeLen+1);
	FILE* stream = fmemopen(b64message, strlen(b64message), "r");

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new_fp(stream, BIO_NOCLOSE);
	bio = BIO_push(b64, bio);
	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	len = BIO_read(bio, *buffer, strlen(b64message));
	(*buffer)[len] = '\0';
	*length = len ;

#ifdef LDEBUG
	printf("output length : %d\n" , len);
#endif

	BIO_free_all(bio);
	fclose(stream);

	return (0); //success
}

int encode_aes128_base64(unsigned char *inbuf , unsigned char *outbuf) {

        EVP_CIPHER_CTX *cten;
	unsigned char     aesdest [AESSIZE] = { '\0' };
        char    *base64EncodeOutput;
        int writelen     = 0 ;
        int enpadlen     = 0 ;

        memset(aesdest , 0x00 , sizeof(aesdest));

        // AES 128
        cten = EVP_CIPHER_CTX_new();

        if ( ! EVP_EncryptInit_ex(cten, EVP_aes_128_cbc(), NULL, (unsigned char*)key, iv) )
                return 11 ;

        if ( ! EVP_EncryptUpdate(cten, aesdest, &writelen, (unsigned char *)inbuf, strlen((const char *)inbuf)) )
                return 12 ;

        if ( ! EVP_EncryptFinal_ex(cten, (unsigned char *)(aesdest + writelen), &enpadlen) )
                return 13 ;

        EVP_CIPHER_CTX_cleanup(cten);

#ifdef LDEBUG
        int i;
        //printf("ENCODE BUF   2   : [%s]\n" , aesdest);
        printf("ENCODE WRITE LEN : [%d]\n" , writelen);
        printf("ENCODE PADD LAST : [%d]\n" , enpadlen);

        printf("* ENCODE BUF\n");
        for(i=0;i<writelen+enpadlen;i++) {
                //printf("[%02d] ; 0x%02x , %c\n" , i , aesdest[i] , aesdest[i]);
                printf("[%02d] ; 0x%02x \n" , i , aesdest[i] );
        }
#endif

        // base64 encode
        Base64Encode((char *)aesdest , writelen+enpadlen , &base64EncodeOutput);

#ifdef LDEBUG
        printf("* ENCODE to Base64 : [%s]\n" , base64EncodeOutput);
#endif

	sprintf((char *)outbuf , "%s"  , base64EncodeOutput);

	return 0 ;
}

int decode_base64_aws128(char *inbuf , char *obuf) {

        EVP_CIPHER_CTX *ctdes;
	unsigned char     aesdest  [AESSIZE] = { '\0' };
        int decode_buf_size = 0 ;
        char* base64DecodeOutput;

        int readlen      = 0 ;
        int rlastlen      = 0 ;

        memset(aesdest , 0x00 , sizeof(aesdest));

        Base64Decode(inbuf , &decode_buf_size , &base64DecodeOutput);

#ifdef LDEBUG
        int i;
        printf("Output size: %d\n", decode_buf_size);
#endif

        // AES decode

        ctdes = EVP_CIPHER_CTX_new();

        if ( ! EVP_DecryptInit_ex(ctdes, EVP_aes_128_cbc(), NULL, (unsigned char*)key, iv) )
                return 21 ;

        if ( ! EVP_DecryptUpdate(ctdes, aesdest, &readlen, (unsigned char *)base64DecodeOutput, decode_buf_size) )
                return 22 ;

        if ( ! EVP_DecryptFinal_ex(ctdes, (unsigned char *)(base64DecodeOutput + readlen), &rlastlen) )
                return 23 ;

        memset(aesdest + readlen + rlastlen, 0x00 , sizeof(aesdest) - readlen - rlastlen);

        EVP_CIPHER_CTX_cleanup(ctdes);

#ifdef LDEBUG
        printf("DECODE BUF   2   : [%s]\n" , aesdest);
        printf("DECODE READ LEN  : [%d]\n" , readlen);
        printf("DECODE READ LAST : [%d]\n" , rlastlen);

        printf("* DECODE BUF\n");
        //for(i=0;i<strlen(inbuf);i++) {
        for(i=0;i<strlen(aesdest);i++) {
                printf("[%02d] ; 0x%02x , %c\n" , i , aesdest[i] , aesdest[i]);
        }
        //printf("[%02d] ; 0x%02x , %c\n" , 4 , aesdest[4] , aesdest[4]);
#endif

	sprintf(obuf , "%s" , aesdest);

	return 0 ;
}

/*
int rawread(char *devname , char *obuf) {
	int rsize , fd , i;
	char buf[WRSIZE] ;

	memset(buf , 0x00 , sizeof(buf));

	fd = open(devname , O_RDONLY | O_DIRECT | O_SYNC );

	if ( fd < 0 ) {
		printf("open error\n");
		return 1 ; 
	}

	if ( lseek(fd,AOFFSET,SEEK_SET) ) {
		rsize = read(fd , buf , sizeof(buf)) ;
		if ( rsize < 0 ) {
			printf("READ ERROR . No.[%d]\n" , errno);
		}
#ifdef LDEBUG
		printf("READ SIZE : %d\n" , rsize);
		for(i=0;i<rsize;i++) {
			if ( i % 10 == 0 )
				printf("\n");
			printf("buf[%d] : 0x%02x" , i ,  buf[i]);
			if ( buf[i] >= 0x32 && buf[i] <= 0x7e )
				printf("(%c)" , buf[i]);
			printf(" , ");
		}
#endif
		sprintf(obuf , "%s" , buf);
	} else {
		close(fd);
		printf("seek error\n");
		return 2 ; 
	}

	close(fd);

	return 0 ;
}

int rawwrite(char *devname , char *wbuf , int wbuf_size) {
	int wsize , fd ;
	int tsize ;
	int blksize ;
	char buf[WRSIZE] ;
	memset(buf , 0x00 , sizeof(buf));
	memcpy(buf , wbuf , wbuf_size);

	fd = open(devname , O_WRONLY | O_DIRECT | O_SYNC);

	if ( fd < 0 ) {
		printf("open error\n");
		return 11 ; 
	}

	if (ioctl(fd, BLKGETSIZE, &blksize) != 0)
		blksize=0;

	if ( blksize == 0 )
		return 15 ; 

	// calc byte
	int wxnum = wbuf_size / blksize + 1 ;

	if ( lseek(fd,AOFFSET,SEEK_SET) ) {
		wsize = write(fd , buf , blksize*wxnum) ;

		//wsize = write(fd , wbuf , wbuf_size) ;
		//tsize = write(fd , 0x00 , 1) ;
#ifdef LDEBUG
		printf("BUF SIZE : %d\n" , wbuf_size);
		printf("WRITE PLAN SIZE : %d\n" , blksize*wxnum);
		printf("WRITE RET  SIZE : %d\n" , wsize);
		//printf("END CODE WRITE SIZE : %d\n" , tsize);
#endif
		if ( wsize < 0 ) {
			printf("Write Error , ErrorNo[%d]\n" , errno);
			return 12 ; 
		}
	} else {
		close(fd);
		printf("seek error\n");
		return 12 ; 
	}

	close(fd);

	return 0 ;
}
*/

int fsenvread(char *devname , char *obuf) {
        int ret;
        int mount_flag = 0;
        int totallen = 0 ;
        int rlen = 0 ;

        int pid = getpid();

        FILE *fp;

        char path[512] ;
        char fpath[1024] ;
	char buf[WRSIZE] ;
        char lbuf[1024] ;

	memset(buf , 0x00 , sizeof(buf));
        memset(path , 0x00 , sizeof(path));
        memset(fpath , 0x00 , sizeof(fpath));

        sprintf(path , "/tmp/._env_%d" , pid);
        sprintf(fpath , "%s/%s" , path , DEFAULT_OFILE);

        if (mkdir(path ,
                S_IRUSR | S_IWUSR | S_IXUSR |
                S_IRGRP | S_IWGRP | S_IXGRP |
                S_IROTH | S_IXOTH | S_IXOTH) != 0) {
                        return 1 ;
        }

        ret = mount(devname , path, "ext4", mount_flag, "");
        if (ret == -1) {
                return 2 ;
        }

	// TODO : Logic
	fp = fopen(fpath, "r");
	if(fp == NULL) {
        	umount(path);
        	remove(path);
		return 3;
	} else {
		while(!feof(fp)){
			memset(lbuf , 0x00 , sizeof(lbuf));
			rlen = fread(lbuf, sizeof(char), sizeof(lbuf) , fp);
			memcpy(&buf[totallen] , lbuf , rlen);
			totallen += rlen ;
		}
		fclose(fp);
		sprintf(obuf , "%s" , buf);
	}

        ret = umount(path);
        if (ret == -1) {
                return 4 ;
        }

        remove(path);

	return 0 ;
}

int fsenvwrite(char *devname , char *wbuf) {
        int ret;
        int mount_flag = 0;

        int pid = getpid();

        FILE *fp;

        char path[512] ;
        char fpath[1024] ;
	char buf[WRSIZE] ;

	memset(buf , 0x00 , sizeof(buf));
        memset(path , 0x00 , sizeof(path));
        memset(fpath , 0x00 , sizeof(fpath));

        sprintf(path , "/tmp/._env_%d" , pid);
        sprintf(fpath , "%s/%s" , path , DEFAULT_OFILE);

        if (mkdir(path ,
                S_IRUSR | S_IWUSR | S_IXUSR |
                S_IRGRP | S_IWGRP | S_IXGRP |
                S_IROTH | S_IXOTH | S_IXOTH) != 0) {
                        return 1 ;
        }

        ret = mount(devname , path, "ext4", mount_flag, "");
        if (ret == -1) {
                return 2 ;
        }

	fp = fopen(fpath, "w");
	if(fp == NULL) {
		return 3;
	} else {
		fprintf(fp , "%s" , wbuf);
		fclose(fp);
	}

        ret = umount(path);
        if (ret == -1) {
                return 4 ;
        }

        remove(path);

	return 0 ;
}

int write_env(char *devname , char *envfile) {


	// Get File Contents
	char buf[WRSIZE] ;
	char lbuf[1024] ;
	unsigned char     enbuf[AESSIZE] = { '\0' };
	int rlen = 0 , totallen = 0 ;
	int ret = 0;
	FILE *fp;

	memset(buf , 0x00 , sizeof(buf));
	memset(enbuf , 0x00 , sizeof(enbuf));

	fp = fopen(envfile, "r");
	if(fp == NULL) {
		return 100;
	} else {
		// Read File
		while(!feof(fp)){
			memset(lbuf , 0x00 , sizeof(lbuf));
			rlen = fread(lbuf, sizeof(char), sizeof(lbuf) , fp);
			memcpy(&buf[totallen] , lbuf , rlen);
			totallen += rlen ;
		}
		fclose(fp);
	}

#ifdef LDEBUG
	printf("DEBUG : %s\n" , buf);
#endif

	ret = encode_aes128_base64((unsigned char *)buf , enbuf) ;
	if ( ret != 0 )
		return ret + 110 ;

#ifdef LDEBUG
	printf("DEBUG ENCODE : %s\n" , enbuf);
#endif

	ret = fsenvwrite(devname , (char *)enbuf);
	if ( ret != 0 )
		return ret + 120 ;

	return ret ;
}

int read_env(char *devname) {
	int ret = 0 ;

	char buf[WRSIZE] ;
	unsigned char     debuf[AESSIZE] = { '\0' };

	memset(buf , 0x00 , sizeof(buf));
	memset(debuf , 0x00 , sizeof(debuf));

	// rawread
	//ret = rawread(devname , buf) ;
	ret = fsenvread(devname , buf) ;
	if ( ret != 0 ) {
		return ret + 200 ;
	}

#ifdef LDEBUG
	printf("RAW READ BASE64 : [%s]\n" , buf);
	printf("RAW READ SIZE   : [%d]\n" , strlen(buf));
#endif

	// decode
	ret = decode_base64_aws128(buf , (char *)debuf) ;
	if ( ret != 0 ) {
		return ret + 210 ;
	}

	// print
	printf("%s" , debuf);

	return ret ;
}

int main(int argc , char *argv[]) {
	int ret = 1 ;

	// Exec Mode Flag
	int mode = 0 ; // 1 : Read , 2 : Write
	int opt ;
	char storage[2048] , envfile[2048]; 

	memset( storage , 0x00 , sizeof(storage) );
	memcpy( storage , DEFAULT_STORAGE , strlen(DEFAULT_STORAGE) );
	memset( envfile , 0x00 , sizeof(envfile) );
	memcpy( envfile , DEFAULT_ENVFILE , strlen(DEFAULT_ENVFILE) );

	opterr = 0;

	while ((opt = getopt(argc, argv, "rws:f:")) != -1) {
		switch (opt) {
			case 'r':
				mode = 1 ;
				break;
			case 'w':
				mode = 2 ;
				break;
			case 's': // Specify Device
				memset( storage , 0x00 , sizeof(storage) );
				memcpy( storage , optarg , strlen(optarg) );
				break;
			case 'f': // Write Envroiment file
				memset( envfile , 0x00 , sizeof(envfile) );
				memcpy( envfile , optarg , strlen(optarg) );
				break;
			default: /* '?' */
				//printf("Usage: %s [-f] [-g] [-h argment] arg1 ...\n", argv[0]);
				break;
		}
	}

#ifdef LDEBUG
	printf("MODE : %d\n" , mode);
	printf("STORAGE : %s\n" , storage);
	if ( mode == 2 )
		printf("Envfile : %s\n" , envfile);
#endif

	switch(mode) {
			case 1: // READ DATA
				ret = read_env(storage);
				break;
			case 2: // WRITE DATA
				ret = write_env(storage , envfile);
				break;
			default:
				// No Action
				ret = 1 ;
				break;
	}

	return ret ;  
}
