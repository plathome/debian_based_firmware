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
 *	usage obsvx1-modem
 *
 *	Init mode 
 *	obsvx1-modem init
 *
 *	Read mode 
 *	obsvx1-modem read
 *
 *	Write mode 
 *	obsvx1-modem write value
 *
 *	value	= 0x00-0xff
 */

#define I2C_NAME "/dev/i2c-5"
#define SLAVE 0x20
#define INIT_MODEM	0x70
#define INIT_OUTPUT	0xFE

#define POWER "power"
#define RESET1 "reset1"
#define RESET2 "reset2"
#define USBRST "usbrst"
#define UARTINI "uartini"
#define AREAIND "areaind"
#define PSHOLD "pshhold"
#define RSTCHK "rstchk"
#define RI "ri"
#define FUPSTS "fupsts"
#define ANT0 "ant0"
#define ANT1 "ant1"
#define MOSIND "mosind"
#define INIT "init"
#define RAW "raw"

#define M_POWER		0x01
#define M_USBRST	0x02
#define M_RESET1	0x04
#define M_RESET2	0x08
#define M_UARTINI	0x80
#define M_AREAIND	0x01
#define M_PSHOLD	0x02
#define M_RSTCHK	0x04
#define M_RI		0x08
#define M_FUPSTS	0x10
#define M_ANT0		0x20
#define M_ANT1		0x40
#define M_MOSIND	0x80

#define U200E	"U200E"
#define UM04	"UM04"
#define KYM11	"KYM11"

enum {
	INPUT0 = 0,
	INPUT1,
	OUTPUT0,
	OUTPUT1,
	POLARITY0,
	POLARITY1,
	CONFIG0,
	CONFIG1,
};

void usage(char *fname)
{
	printf("%s option\n", fname);
	printf("option :\n");
	printf("power  [high|low]: change high/low power of module1\n");
	printf("reset1 [high|low]: change high/low reset of module1\n");
	printf("reset2 [high|low]: change high/low reset of module2\n");
	printf("usbrst [high|low]: change high/low reset of usb hub\n");
	printf("init             : initialize gpio\n");
	printf("raw              : read raw data (DEBUG)\n");
#if 0
	printf("\nfor KYM1x\n");
	printf("uartini          : initialize uart\n");
	printf("areaind          : area ind\n");
	printf("pshhold          : psh hold\n");
	printf("rstchk           : rst chk\n");
	printf("ri               : ri\n");
	printf("fupsts           : fupsts\n");
	printf("ant0             : antinf_0\n");
	printf("ant1             : antinf_1\n");
	printf("mosind           : mos_ind\n");
#endif
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

int read_gpio(unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if((val[0] = i2c_smbus_read_byte_data(fd, INPUT0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[1] = i2c_smbus_read_byte_data(fd, INPUT1)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int write_gpio(unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if(i2c_smbus_write_byte_data(fd, OUTPUT0, val[0]) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

unsigned short init_gpio(void)
{
	int fd;
	unsigned char val;
#if 0
	char mname[16];
	FILE *fp;
#endif

#if 0
	if((fp = popen("/usr/sbin/obsiot-modem.sh", "r")) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	fgets(mname, sizeof(mname)-1, fp);
	pclose(fp);
#endif

	if((fd = open_device()) < 0)
		return -1;

	if(i2c_smbus_write_byte_data(fd, CONFIG0, INIT_MODEM) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val = i2c_smbus_read_byte_data(fd, CONFIG0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
#if 0
	if(strncmp(KYM11, mname, sizeof(KYM11)) == 0){
		if(i2c_smbus_write_byte_data(fd, OUTPUT0, INIT_OUTPUT) == -1){
			printf("ERR%d: %s\n", __LINE__, strerror(errno));
			close(fd);
			return -1;
		}
	}
#endif
	close(fd);
	printf("%02x\n", val);

	return 0;
}

int read_register(unsigned char mask)
{
	unsigned char val[2];

	if(read_gpio(val) < 0){
		return -1;
	}
	if(val[0] & mask)
		return 1;
	else
		return 0;
}

int write_register(char *flag, unsigned char mask)
{
	unsigned char val[2];

	if(read_gpio(val) < 0)
		return -1;

	if(strcmp("high", flag) == 0)
		val[0] |= mask;
	else if(strcmp("low", flag) == 0)
		val[0] &= ~mask;
	else
		return -1;

	if(write_gpio(val) < 0)
		return -1;

	return read_register(mask);
}

int read_config(unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if((val[0] = i2c_smbus_read_byte_data(fd, CONFIG0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[1] = i2c_smbus_read_byte_data(fd, CONFIG1)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int read_output(unsigned char *val)
{
	int fd;

	if((fd = open_device()) < 0)
		return -1;

	if((val[0] = i2c_smbus_read_byte_data(fd, OUTPUT0)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	if((val[1] = i2c_smbus_read_byte_data(fd, OUTPUT1)) == -1){
		printf("ERR%d: %s\n", __LINE__, strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int main(int ac, char *av[])
{
	int ret=0;

	if(ac != 2 && ac != 3){
		usage(av[0]);
		return -1;
	}
	if(strncmp(INIT, av[1], strlen(INIT)) == 0){
		if(init_gpio() == -1)
			return -1;
	}
	else if(strncmp(POWER, av[1], strlen(POWER)) == 0){
		if(ac == 2){
			if((ret = read_register(M_POWER)) == -1)
				return -1;
			printf("%d\n", ret);
		}
		else{
			if((ret = write_register(av[2], M_POWER)) == -1)
				return -1;
		}
	}
	else if(strncmp(RESET1, av[1], strlen(RESET1)) == 0){
		if(ac == 2){
			if((ret = read_register(M_RESET1)) == -1)
				return -1;
			printf("%d\n", ret);
		}
		else{
			if((ret = write_register(av[2], M_RESET1)) == -1)
				return -1;
		}
	}
	else if(strncmp(RESET2, av[1], strlen(RESET2)) == 0){
		if(ac == 2){
			if((ret = read_register(M_RESET2)) == -1)
				return -1;
			printf("%d\n", ret);
		}
		else{
			if((ret = write_register(av[2], M_RESET2)) == -1)
				return -1;
		}
	}
	else if(strncmp(USBRST, av[1], strlen(USBRST)) == 0){
		if(ac == 2){
			if((ret = read_register(M_USBRST)) == -1)
				return -1;
			printf("%d\n", ret);
		}
		else{
			if((ret = write_register(av[2], M_USBRST)) == -1)
				return -1;
		}
	}
	else if(strncmp(UARTINI, av[1], strlen(UARTINI)) == 0){
	}
	else if(strncmp(AREAIND, av[1], strlen(AREAIND)) == 0){
	}
	else if(strncmp(PSHOLD, av[1], strlen(PSHOLD)) == 0){
	}
	else if(strncmp(RSTCHK, av[1], strlen(RSTCHK)) == 0){
	}
	else if(strncmp(RI, av[1], strlen(RI)) == 0){
	}
	else if(strncmp(FUPSTS, av[1], strlen(FUPSTS)) == 0){
	}
	else if(strncmp(ANT0, av[1], strlen(ANT0)) == 0){
	}
	else if(strncmp(ANT1, av[1], strlen(ANT1)) == 0){
	}
	else if(strncmp(MOSIND, av[1], strlen(MOSIND)) == 0){
	}
	else if(strncmp(RAW, av[1], strlen(RAW)) == 0){
		unsigned char val[2];

		if(read_gpio(val) < 0)
			return -1;
		printf("input: %02x %02x\n", val[1], val[0]);
		if(read_config(val) < 0)
			return -1;
		printf("config: %02x %02x\n", val[1], val[0]);
		if(read_output(val) < 0)
			return -1;
		printf("output: %02x %02x\n", val[1], val[0]);
	}
	else{
		usage(av[0]);
		return -1;
	}

	return ret;
}

