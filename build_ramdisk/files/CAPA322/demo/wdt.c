#include <stdio.h>
#include <unistd.h>
#include "libaxio.h"

int main()
{
	int i;
	int second=0;
	//int minute=1;
	unsigned int WDT_timeout=10;
	int unused=0x00;
	printf("Enable the Watch Dog Timer.\n\n");
	printf("If you want to reset OS by WDT, please click ctrl+c and wait a second.\n\n");

	AXBoardSetIndexValue(EAPI_AX_HWMON_WDT_ENABLE,second,WDT_timeout);
	for(i=10; i>=5; i--) {
	    printf("%d sec.....\n",i);
	    usleep(1000000); //delay 1 sec
	}


	AXBoardSetIndexValue(EAPI_AX_HWMON_WDT_RELOAD,unused,unused);
	printf("Reload WDT timer .....\n");
	for(i=10; i>=5; i--) {
	    printf("%d sec.....\n",i);
	    usleep(1000000); //delay 1 sec
	}


	printf("Disable the Watch Dog Timer\n\n");
	AXBoardSetIndexValue(EAPI_AX_HWMON_WDT_STOP,unused,unused);


    return 0;
}

