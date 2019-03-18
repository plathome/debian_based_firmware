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

int readPortConfig(DWORD devnum, DUAL_PORT_CONFIG *portConfig)
{
	HANDLE hd;
	CP210x_STATUS sta;

	if((sta = CP210x_Open(devnum, &hd)) != CP210x_SUCCESS){
		printf("no device found!(status=%x)\n", sta);
		return -1;
	}
	if((sta = CP210x_GetDualPortConfig(hd, portConfig)) != CP210x_SUCCESS){
		printf("GetDualPortConfig error.(status=%x)\n", sta);
		CP210x_Close(hd);
		return -1;
	}
	CP210x_Close(hd);

	return 0;
}


int writePortConfig(DWORD devnum)
{
	HANDLE hd;
	DUAL_PORT_CONFIG portConfig;
	CP210x_STATUS sta;

	if((sta = CP210x_Open(devnum, &hd)) != CP210x_SUCCESS){
		printf("no device found!(status=%x)\n", sta);
		return -1;
	}
	if((sta = CP210x_GetDualPortConfig(hd, &portConfig)) != CP210x_SUCCESS){
		printf("GetDualPortConfig error.(status=%x)\n", sta);
		CP210x_Close(hd);
		return -1;
	}

	/* check default value */
	if(portConfig.Mode != 0x5151
	|| portConfig.Reset_Latch != 0xfefe
	|| portConfig.Suspend_Latch != 0xfefe
	|| portConfig.EnhancedFxn_ECI != 0x10
	|| portConfig.EnhancedFxn_SCI != 0x10
	|| portConfig.EnhancedFxn_Device != 0x10){
		printf("DualPortConfig isn't default value!(m=%x r=%x s=%x e=%x s=%x d=%x)\n",
		portConfig.Mode, portConfig.Reset_Latch,
		portConfig.Suspend_Latch, portConfig.EnhancedFxn_ECI,
		portConfig.EnhancedFxn_SCI, portConfig.EnhancedFxn_Device);
		CP210x_Close(hd);
		return -1;
	}

	portConfig.Mode = 0x5951;
	portConfig.Reset_Latch = 0xf6fe;
	portConfig.Suspend_Latch = 0xf6fe;
	portConfig.EnhancedFxn_ECI |= EF_GPIO_1_RS485_ECI | EF_RS485_INVERT;
	portConfig.EnhancedFxn_SCI = 0x10;
	portConfig.EnhancedFxn_Device = 0x10;
#if 1
	if((sta = CP210x_SetDualPortConfig(hd, &portConfig)) != CP210x_SUCCESS){
		printf("SetDualPortConfig error.(status=%x)\n", sta);
		CP210x_Close(hd);
		return -1;
	}
#else
	printf("Port Config = %x %x %x %x %x %x\n", portConfig.Mode,
			portConfig.Reset_Latch, portConfig.Suspend_Latch,
			portConfig.EnhancedFxn_ECI, portConfig.EnhancedFxn_SCI,
			portConfig.EnhancedFxn_Device);
#endif

	CP210x_Close(hd);
	return 0;
}

int main(int ac, char* av[])
{
	DUAL_PORT_CONFIG portConfig;
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
		printf("Port Config = %x %x %x %x %x %x\n", portConfig.Mode,
				portConfig.Reset_Latch, portConfig.Suspend_Latch,
				portConfig.EnhancedFxn_ECI, portConfig.EnhancedFxn_SCI,
				portConfig.EnhancedFxn_Device);
	}
	else if(av[3][0] == 'w'){
		if(writePortConfig((DWORD)val) < 0){
			return -1;
		}
		if(readPortConfig((DWORD)val, &portConfig) < 0){
			printf("Port Config read error\n");
			return -1;
		}
		printf("Port Config = %x %x %x %x %x %x\n", portConfig.Mode,
				portConfig.Reset_Latch, portConfig.Suspend_Latch,
				portConfig.EnhancedFxn_ECI, portConfig.EnhancedFxn_SCI,
				portConfig.EnhancedFxn_Device);
	}
	else{
		printf("Bad option.(%s)\n", av[1]);
		return -1;
	}

	return 0;
}

