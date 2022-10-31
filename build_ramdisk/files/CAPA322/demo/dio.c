#include <stdio.h>
#include <unistd.h>
#include "libaxio.h"

int main()
{
	unsigned int hi_low;
	int i,device=0;
	usleep(100000);
	
	AXBoardSetIndexValue(EAPI_AX_HWMON_MAIN_DIO_DIR,device,0x0F);
	usleep(100000);
	AXBoardGetIndexValue(EAPI_AX_HWMON_MAIN_DIO_DIR,device,&hi_low);
	printf("dio mode value[%02x]\n",hi_low);
	printf("This is a sample, DIO0~3 set to DI and DIO4~7 set to DO. \n");
	printf("Set DO4~7 to high\n");
	for(i=0;i<4;i++)
	{
		AXBoardSetValue(EAPI_AX_HWMON_DO4+i,1);
	}

	for(i=0;i<4;i++)
        {
                EApiBoardGetValue(EAPI_AX_HWMON_DI0+i,&hi_low);
                printf("DI%d : [%d]\n",i,hi_low);
        }

	for(i=0;i<4;i++)
	{
		EApiBoardGetValue(EAPI_AX_HWMON_DO4+i,&hi_low);
		printf("DO%d : [%d]\n",i+4,hi_low);
	}
	
	printf("Set DO4~7 to low\n");
	for(i=0;i<4;i++)
                AXBoardSetValue(EAPI_AX_HWMON_DO4+i,0);
        	
        for(i=0;i<4;i++)
        {
                EApiBoardGetValue(EAPI_AX_HWMON_DI0+i,&hi_low);
                printf("DI%d : [%d]\n",i,hi_low);
        }
        
	for(i=0;i<4;i++)
        {
                EApiBoardGetValue(EAPI_AX_HWMON_DO4+i,&hi_low);
                printf("DO%d : [%d]\n",i+4,hi_low);
        }
	return 0;
}

