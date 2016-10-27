#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
 *	usage obsvx1-gpio
 *
 *	Init mode 
 *	obsvx1-gpio init
 *
 *	Read mode 
 *	obsvx1-gpio read
 *
 *	Write mode 
 *	obsvx1-gpio write value
 *
 *	value	= 0x00-0xff
 */

#define I2C_NAME "/dev/i2c-5"
#define SLAVE 0x70

#define QUIET "-q"
#define INIT "init"
#define READ "read"
#define WRITE "write"

enum {
	INPUT = 0,
	OUTPUT,
	POLARITY,
	CONFIG,
};

static int quiet;

void usage(char *fname)
{
	printf("%s [option] command\n", fname);
	printf("\n");
	printf("option\n");
	printf("-q : quiet mode \n");
	printf("command\n");
	printf("init init_value : initialize gpio\n");
	printf("\tinit_value  0:out 1:in\n");
	printf("\texp : %s init 01010101\n", fname);
	printf("read : read gpio,  LSB format\n");
	printf("write : write gpio,  LSB format\n");
	printf("\texp : %s write 00001111\n", fname);
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

int read_gpio(void)
{
	int fd;
	int val;

	if((fd = open_device()) < 0)
		return -1;

	if((val = i2c_smbus_read_byte_data(fd, INPUT)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return val;
}

int write_gpio(char *io)
{
	int fd;
	int i;
	unsigned char val = 0;

	if(io == NULL){
		printf("ERR%d: parameter error\n", __LINE__);
		return -1;
	}
	if(strlen(io) != 8){
		printf("ERR%d: parameter error\n", __LINE__);
		return -1;
	}
	for(i=0;i<8;i++){
		if(io[i] != '0' && io[i] != '1'){
			printf("ERR%d: parameter error\n", __LINE__);
			return -1;
		}
		if(io[i] - '0')
			val |= (1 << (7 -i)); 
	}

	if((fd = open_device()) < 0)
		return -1;

	if(i2c_smbus_write_byte_data(fd, OUTPUT, val) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

unsigned short init_gpio(char *io)
{
	int fd;
	int i;
	unsigned char val = 0;

	if(io == NULL){
		printf("ERR%d: parameter error\n", __LINE__);
		return -1;
	}
	if(strlen(io) != 8){
		printf("ERR%d: parameter error\n", __LINE__);
		return -1;
	}
	for(i=0;i<8;i++){
		if(io[i] != '0' && io[i] != '1'){
			printf("ERR%d: parameter error\n", __LINE__);
			return -1;
		}
		if(io[i] - '0')
			val |= (1 << (7 -i)); 
	}

	if((fd = open_device()) < 0)
		return -1;

	if(i2c_smbus_write_byte_data(fd, CONFIG, val) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val = i2c_smbus_read_byte_data(fd, CONFIG)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return val;
}

int main(int ac, char *av[])
{
	int ret=0;

	if(ac < 2 || ac > 4){
		usage(av[0]);
		return -1;
	}

	if(strncmp(QUIET, av[1], strlen(QUIET)) == 0)
		quiet = 1;

	if(strncmp(INIT, av[1], strlen(INIT)) == 0){
		if((ret = init_gpio(av[2])) == -1)
			return -1;
		if(!quiet)
			printf("%02x\n", ret);
	}
	else if(strncmp(READ, av[1], strlen(READ)) == 0){
		if((ret = read_gpio()) < 0)
			return -1;
		if(!quiet)
			printf("%02x\n", ret);
	}
	else if(strncmp(WRITE, av[1], strlen(WRITE)) == 0){
		if(write_gpio(av[2]) < 0)
			return -1;
		if((ret = read_gpio()) < 0)
			return -1;
		if(!quiet)
			printf("%02x\n", ret);
	}
	else{
		usage(av[0]);
		return -1;
	}

	return ret;
}
