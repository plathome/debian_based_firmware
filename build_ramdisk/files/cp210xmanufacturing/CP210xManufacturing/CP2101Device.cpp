/*
 * CP2101Device.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2101Device.h"
#include "CP210xSupportFunctions.h"

CCP2101Device::CCP2101Device(libusb_device_handle* h) {
    m_handle = h;
    m_partNumber = 0x01;
    maxSerialStrLen = CP210x_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP210x_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2101Device::GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE lpbLength, BOOL bConvertToASCII) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::GetLockValue(LPBYTE lpbLockValue) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetManufacturerString(LPVOID lpvManufacturer, BYTE bLength, BOOL bConvertToUnicode) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2101Device::SetLockValue() {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}
