#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "CP210xManufacturing.h"

#define VID 0x10c4
#define PID 0xea60

void PrintBuffer(BYTE* buffer, DWORD len) {
    for (DWORD i = 0; i < len; i++) {
        printf("%02x", buffer[i]);

        if (i % 16 == 15)
            printf("\n");
        else if (i < len - 1)
            printf(" ");
    }
}

void Test() {
    CP210x_STATUS status;
    DWORD dwNumDevices = 0;

    status = CP210x_GetNumDevices(&dwNumDevices);

    if (status == CP210x_SUCCESS) {
        printf("Found %d devices\n", dwNumDevices);
        for (DWORD i = 0; i < dwNumDevices; i++) {
            //TODO
            CP210x_DEVICE_STRING str;
            CP210x_GetProductString(i, str, 0xFF);
            printf("\nDevice %d, %s\n", i, str);
            HANDLE h;
            if (CP210x_Open(i, &h) == CP210x_SUCCESS) {
                BYTE partNum;
                WORD vid, pid;
                BOOL selfPower;
                BYTE maxPower;
                WORD version;
                WORD flushBuf;
                BYTE lockValue;
                BYTE ifc;
                BYTE ifc0Mode, ifc1Mode;
                PORT_CONFIG portConfig;
                DUAL_PORT_CONFIG dualPortConfig;
                QUAD_PORT_CONFIG quadPortConfig;
                BAUD_CONFIG_DATA baudConfig;
                CP210x_PRODUCT_STRING productString;
                CP210x_SERIAL_STRING serialString;
                CP210x_MANUFACTURER_STRING manufacturerString;
                CP2105_INTERFACE_STRING cp2105InterfaceString;
                CP2108_INTERFACE_STRING cp2108InterfaceString;
                BYTE length;

                status = CP210x_GetPartNumber(h, &partNum);
                printf("\nstatus = %X, Part Number = %02X\n", status, partNum);

                status = CP210x_GetDeviceVid(h, &vid);
                printf("status = %X, Vendor ID = %04X\n", status, vid);

                status = CP210x_GetDevicePid(h, &pid);
                printf("status = %X, Product ID = %04X\n", status, pid);

                status = CP210x_GetSelfPower(h, &selfPower);
                printf("status = %X, SelfPower = %02X\n", status, selfPower);

                status = CP210x_GetMaxPower(h, &maxPower);
                printf("status = %X, MaxPower = %02X\n", status, maxPower);

                status = CP210x_GetDeviceVersion(h, &version);
                printf("status = %X, Version = %04X\n", status, version);

                status = CP210x_GetFlushBufferConfig(h, &flushBuf);
                printf("status = %X, Flush Config = %04X\n", status, flushBuf);

                status = CP210x_GetDeviceProductString(h, &productString, &length, true);
                printf("status = %X, Product String = %s\n", status, productString);

                status = CP210x_GetDeviceSerialNumber(h, &serialString, &length, true);
                printf("status = %X, Serial String = %s\n", status, serialString);

                status = CP210x_GetDeviceManufacturerString(h, &manufacturerString, &length, true);
                printf("status = %X, Manufacturer String = %s\n", status, manufacturerString);

                ifc = 0;
                status = CP210x_GetDeviceInterfaceString(h, ifc, &cp2105InterfaceString, &length, true);
                printf("status = %X, CP2105 Interface %d String = %s\n", status, ifc, cp2105InterfaceString);
                ifc = 1;
                status = CP210x_GetDeviceInterfaceString(h, ifc, &cp2105InterfaceString, &length, true);
                printf("status = %X, CP2105 Interface %d String = %s\n", status, ifc, cp2105InterfaceString);

                ifc = 0;
                status = CP210x_GetDeviceInterfaceString(h, ifc, &cp2108InterfaceString, &length, true);
                printf("status = %X, CP2108 Interface %d String = %s\n", status, ifc, cp2108InterfaceString);
                ifc = 1;
                status = CP210x_GetDeviceInterfaceString(h, ifc, &cp2108InterfaceString, &length, true);
                printf("status = %X, CP2108 Interface %d String = %s\n", status, ifc, cp2108InterfaceString);
                ifc = 2;
                status = CP210x_GetDeviceInterfaceString(h, ifc, &cp2108InterfaceString, &length, true);
                printf("status = %X, CP2108 Interface %d String = %s\n", status, ifc, cp2108InterfaceString);
                ifc = 3;
                status = CP210x_GetDeviceInterfaceString(h, ifc, &cp2108InterfaceString, &length, true);
                printf("status = %X, CP2108 Interface %d String = %s\n", status, ifc, cp2108InterfaceString);

                status = CP210x_GetDeviceMode(h, &ifc0Mode, &ifc1Mode);
                printf("status = %X, Ifc0Mode = %02X, Ifc1Mode = %02X\n", status, ifc0Mode, ifc1Mode);

                status = CP210x_GetPortConfig(h, &portConfig);
                printf("status = %X, Port Config =\n", status);

                status = CP210x_GetDualPortConfig(h, &dualPortConfig);
                printf("status = %X, Dual Port Config =\n", status);

                status = CP210x_GetQuadPortConfig(h, &quadPortConfig);
                printf("status = %X, Quad Port Config =\n", status);

                status = CP210x_GetBaudRateConfig(h, baudConfig);
                printf("status = %X, Baud Rate Config =\n", status);

                status = CP210x_GetLockValue(h, &lockValue);
                printf("status = %X, Lock Value = %02X\n", status, lockValue);
               
                CP210x_Close(h);
            }
        }
    } else {
        printf("No devices found!\n");
    }
}

int main(int argc, char* argv[]) {
    Test();

    return 0;
}
