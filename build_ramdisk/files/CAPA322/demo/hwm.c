#include <stdio.h>
#include <unistd.h>
#include "libaxio.h"

int main()
{
	int xData;
    	unsigned int read_v;	

	xData=EApiBoardGetValue(EAPI_ID_HWMON_VOLTAGE_VBAT ,&read_v);//
        if (xData ==0 ) printf("Read VBAT volatage is %2.3f V\n",(float)read_v/100);
        else            printf("This Board not support VBAT monitor function\n");
        
        xData=EApiBoardGetValue(EAPI_ID_HWMON_VOLTAGE_3V3  ,&read_v);//
        if (xData ==0 ) printf("Read +3.3V_SBY  volatage is %2.3f V\n",(float)read_v/100);
        else            printf("This Board not support +3.3V_SBY monitor function\n");
        
        xData=EApiBoardGetValue(EAPI_ID_HWMON_VOLTAGE_5V0 ,&read_v);//
        if (xData ==0 ) printf("Read +5V  volatage is %2.3f V\n",(float)read_v/100);
        else            printf("This Board not support +5V monitor function\n");


	xData=EApiBoardGetValue(EAPI_ID_HWMON_CPU_TEMP ,&read_v);
    	if (xData ==0 ) printf("Read CPU Temp. is %d degree C\n",read_v);
    	else            printf("This Board not support CPU Temp. monitor function\n");	

	xData=EApiBoardGetValue(EAPI_ID_HWMON_SYSTEM1_TEMP ,&read_v);
        if (xData ==0 ) printf("Read System Temp. is %d degree C\n",read_v);
        else            printf("This Board not support System Temp. monitor function\n");
        
        xData=EApiBoardGetValue(EAPI_ID_HWMON_FAN_CPU ,&read_v);
        if (xData ==0 ) printf("Read CPU FAN. is %d  RPM\n",read_v);
        else            printf("This Board not support CPU FAN. monitor function\n");
        
	return 1;
}
