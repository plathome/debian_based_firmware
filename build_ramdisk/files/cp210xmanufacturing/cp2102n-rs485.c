#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <CP210xManufacturing.h>

#define DEVNUM "-d"

void usage(char *fname)
{
	printf("RS485 configuration tool for OBSVX1\n");
	printf("%s -d devnum [w]\n", fname);
	printf("devnum\t: cp210x device number\n");
	printf("w\t: write one time flash\n");
}

int readPortConfig(DWORD devnum, PORT_CONFIG *portConfig)
{
	HANDLE hd;
	CP210x_STATUS sta;

	if((sta = CP210x_Open(devnum, &hd)) != CP210x_SUCCESS){
		printf("no device found!(status=%x)\n", sta);
		return -1;
	}
	if((sta = CP210x_GetPortConfig(hd, portConfig)) != CP210x_SUCCESS){
		printf("GetPortConfig error.(status=%x)\n", sta);
		CP210x_Close(hd);
		return -1;
	}
	CP210x_Close(hd);

	return 0;
}

int writePortConfig(DWORD devnum)
{
	HANDLE hd;
	PORT_CONFIG portConfig;
	CP210x_STATUS sta;

	if((sta = CP210x_Open(devnum, &hd)) != CP210x_SUCCESS){
		printf("no device found!(status=%x)\n", sta);
		return -1;
	}
	if((sta = CP210x_GetPortConfig(hd, &portConfig)) != CP210x_SUCCESS){
		printf("GetPortConfig error.(status=%x)\n", sta);
		CP210x_Close(hd);
		return -1;
	}

	if(portConfig.Mode != 0xf054 || portConfig.Reset_Latch != 0x3fff
		|| portConfig.Suspend_Latch != 0x3fff || portConfig.EnhancedFxn != 0x10){
		printf("PortConfig isn't default value!(m=%x r=%x s=%x e=%x)\n",
		portConfig.Mode, portConfig.Reset_Latch,
		portConfig.Suspend_Latch, portConfig.EnhancedFxn);
		CP210x_Close(hd);
		return -1;
	}

	portConfig.Mode = 0xc454;
	portConfig.Reset_Latch = 0xfff;
	portConfig.Suspend_Latch = 0xfff;
	portConfig.EnhancedFxn |= EF_GPIO_2_RS485 | EF_RS485_INVERT;
#if 1
	if((sta = CP210x_SetPortConfig(hd, &portConfig)) != CP210x_SUCCESS){
		printf("SetPortConfig error.(status=%x)\n", sta);
		CP210x_Close(hd);
		return -1;
	}
#else
	printf("Port Config = %x %x %x %x\n", portConfig.Mode,
			portConfig.Reset_Latch, portConfig.Suspend_Latch,
			portConfig.EnhancedFxn);
#endif

	CP210x_Close(hd);
	return 0;
}

int main(int ac, char* av[])
{
	PORT_CONFIG portConfig;
	long val;
	char *ep;

	if(ac != 3 && ac != 4){
		usage(av[0]);
		return -1;
	}

	if(strncmp(av[1], DEVNUM, sizeof(DEVNUM)) != 0){
		printf("Bad option.(%s)\n", av[1]);
		return -1;
	}

	val = strtol(av[2], &ep, 10);
	if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
											|| (errno != 0 && val == 0)){
		perror("strtol");
		return -1;
	}
	if(ep == av[2]){
		printf("No digits were found\n");
		return -1;
	}

	if(av[3] == NULL){
		if(readPortConfig((DWORD)val, &portConfig) < 0){
			printf("Port Config read error\n");
			return -1;
		}
		printf("Port Config = %x %x %x %x\n", portConfig.Mode,
				portConfig.Reset_Latch, portConfig.Suspend_Latch,
				portConfig.EnhancedFxn);
	}
	else if(av[3][0] == 'w'){
		if(writePortConfig((DWORD)val) < 0){
			return -1;
		}
		if(readPortConfig((DWORD)val, &portConfig) < 0){
			printf("Port Config read error\n");
			return -1;
		}
		printf("Port Config = %x %x %x %x\n", portConfig.Mode,
				portConfig.Reset_Latch, portConfig.Suspend_Latch,
				portConfig.EnhancedFxn);
	}
	else{
		printf("Bad option.(%s)\n", av[1]);
		return -1;
	}

	return 0;
}

