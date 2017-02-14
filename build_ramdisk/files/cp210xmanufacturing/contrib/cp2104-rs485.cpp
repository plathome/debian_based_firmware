/*
 * Karl Palsson, <karlp@remake.is> 2015
 * Based on the provided "main.cpp" file
 * License of _this_ considered to be your choice of BSD 2 clause, MIT, X11
 * or ISC lisenses.
 */
#include <getopt.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "CP210xManufacturing.h"

#define VID 0x10c4
#define PID 0xea60

struct app {
	bool dump;
	bool write;
	int dev_idx;
};

void PrintBuffer(BYTE* buffer, DWORD len)
{
	for (DWORD i = 0; i < len; i++) {
		printf("%02x", buffer[i]);

		if (i % 16 == 15)
			printf("\n");
		else if (i < len - 1)
			printf(" ");
	}
}

int write_rs485_invert(int dev_idx)
{
	CP210x_STATUS status;
	CP210x_DEVICE_STRING str;
	int rc = 0;
	int confirm;

	status = CP210x_GetProductString(dev_idx, str, CP210x_RETURN_FULL_PATH);
	if (status != CP210x_SUCCESS) {
		printf("Device %d doesn't seem to be compatible or wasn't found...\n", dev_idx);
		return -1;
	}
	printf("\nDevice %d, %s\n", dev_idx, str);
	HANDLE h;
	status = CP210x_Open(dev_idx, &h);
	if (status != CP210x_SUCCESS) {
		printf("Unable to open device %d: %d\n", dev_idx, status);
		rc = -1;
		goto out;
	}
	BYTE partNum;
	PORT_CONFIG portConfig;

	status = CP210x_GetPartNumber(h, &partNum);
	printf("\nstatus = %X, Part Number = %02X\n", status, partNum);
	// Thanks silabs, not actually a "version" response
	if (partNum != CP210x_CP2104_VERSION) {
		printf("Device given is not a cp2104\n");
		rc = -1;
		goto out;
	}
	status = CP210x_GetPortConfig(h, &portConfig);
	printf("status = %X, EXISTING Port Config = \n{mode=%#x, reset_latch=%#x, suspend_latch=%#x, enh_fxn=%#x}\n",
		status, portConfig.Mode, portConfig.Reset_Latch, portConfig.Suspend_Latch, portConfig.EnhancedFxn);
	portConfig.EnhancedFxn |= EF_GPIO_2_RS485 | EF_RS485_INVERT;
	printf("status = %X, TO BE WRITTEN Port Config = \n{mode=%#x, reset_latch=%#x, suspend_latch=%#x, enh_fxn=%#x}\n",
		status, portConfig.Mode, portConfig.Reset_Latch, portConfig.Suspend_Latch, portConfig.EnhancedFxn);
	printf("Should we continue? This will attempt to write to OTP ROM!\n");
	printf("\tY to confirm, anything else to abort\n");
	confirm = getchar();
	if (confirm == 'Y') {
		printf("I really hope the author of this code knew what they were doing....\n");
		//CP210x_SetSelfPower(h, true);  // Should do this for correctness?
		status = CP210x_SetPortConfig(h, &portConfig);
		if (status != CP210x_SUCCESS) {
			printf("crap, SetPortConfig failed with code: %d\n", status);
			rc = -1;
			goto out;
		}
		printf("Ok, it _appeared_ to work...\n");
	} else {
		printf("No problem, skipping write\n");
	}
	rc = 0;
out:
	CP210x_Close(h);
	return rc;
}

void dump_device(int i)
{
	CP210x_STATUS status;
	CP210x_DEVICE_STRING str;

	status = CP210x_GetProductString(i, str, CP210x_RETURN_FULL_PATH);
	if (status != CP210x_SUCCESS) {
		printf("Device %d doesn't seem to be compatible or wasn't found...\n", i);
		return;
	}
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
		BYTE ifc0Mode, ifc1Mode;
		PORT_CONFIG portConfig;
		BAUD_CONFIG_DATA baudConfig;
		CP210x_PRODUCT_STRING productString;
		CP210x_SERIAL_STRING serialString;
		CP210x_MANUFACTURER_STRING manufacturerString;
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

		status = CP210x_GetDeviceMode(h, &ifc0Mode, &ifc1Mode);
		printf("status = %X, Ifc0Mode = %02X, Ifc1Mode = %02X\n", status, ifc0Mode, ifc1Mode);

		status = CP210x_GetPortConfig(h, &portConfig);
		printf("status = %X, Port Config = {mode=%#x, reset_latch=%#x, suspend_latch=%#x, enh_fxn=%#x}\n",
			status, portConfig.Mode, portConfig.Reset_Latch, portConfig.Suspend_Latch, portConfig.EnhancedFxn);

		status = CP210x_GetBaudRateConfig(h, baudConfig);
		printf("status = %X, Baud Rate Config =\n", status);
		for (int kk = 0; kk < NUM_BAUD_CONFIGS; kk++) {
			printf("\t{baudgen=%d, baudrate=%d, prescaler=%d, timer0reload=%d}\n",
				baudConfig[0].BaudGen, baudConfig[0].BaudRate, baudConfig[0].Prescaler, baudConfig[0].Timer0Reload);
		}

		status = CP210x_GetLockValue(h, &lockValue);
		printf("status = %X, Lock Value = %02X\n", status, lockValue);

		CP210x_Close(h);
	}

}

void dump_all_devices(struct app *st)
{
	CP210x_STATUS status;
	DWORD dwNumDevices = 0;

	status = CP210x_GetNumDevices(&dwNumDevices);

	if (status == CP210x_SUCCESS) {
		printf("Found %d usb devices\n", dwNumDevices);
		for (DWORD i = 0; i < dwNumDevices; i++) {
			if(i == 5){
				i++;
				continue;
			}
			dump_device(i);
		}
	} else {
		printf("No usb devices found!\n");
	}
}

int parse_options(int argc, char** argv, struct app *st)
{
	static struct option long_options[] = {
		{"device", required_argument, NULL, 'd'},
		{"list", no_argument, NULL, 'l'},
		{"write", no_argument, NULL, 'w'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0},
	};

	int option_index = 0;
	int c, q;
	while ((c = getopt_long(argc, argv, "d:lwh", long_options, &option_index)) != -1) {
		switch (c) {
		case 0:
			printf("XXXXX Shouldn't really normally come here, only if there's no corresponding option\n");
			printf("option %s", long_options[option_index].name);
			if (optarg) {
				printf(" with arg %s", optarg);
			}
			printf("\n");
			break;
		case 'h':
			printf("%s - usage:\n\n", argv[0]);
			printf("  -d <index>, --device=index\n");
			printf("\t\t\tspecify a particular device to work on\n");
			printf("  -l, --list\n");
			printf("\t\t\tList the existing config of a device or devices\n");
			printf("  -w, --write\n");
			printf("\t\t\tAttempt to set the device to rs485 mode+invert (BE CAREFUL!)\n");
			printf("  -h, --help\t\tPrint this help\n");
			printf("\nWith no arguments, list mode will run.\n");
			exit(EXIT_SUCCESS);
			break;
		case 'd':
			sscanf(optarg, "%i", &q);
			st->dev_idx = q;
			break;
		case 'l':
			st->dump = true;
			break;
		case 'w':
			st->write = true;
			break;
		case '?':
			printf("Busted arguments, aborting\n");
			exit(1);
			break;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}
	return 0;
}

int main(int argc, char* argv[])
{
	struct app state = {0};
	state.dev_idx = -1;
	state.dump = true;
	printf("Karl's test testy version\n");
	parse_options(argc, argv, &state);
	if (state.dump) {
		if (state.dev_idx >= 0) {
			dump_device(state.dev_idx);
		} else {
			dump_all_devices(&state);
		}
	}
	if (state.write) {
		if (state.dev_idx < 0) {
			printf("Must specify a device to write!\n");
			exit(1);
		}
		write_rs485_invert(state.dev_idx);
	}

	return 0;
}
