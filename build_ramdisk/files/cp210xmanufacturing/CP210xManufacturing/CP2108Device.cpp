/*
 * CP2108Device.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2108Device.h"
#include "CP210xSupportFunctions.h"

CCP2108Device::CCP2108Device(libusb_device_handle* h) {
    m_handle = h;
    m_partNumber = 0x08;
    maxSerialStrLen = CP2108_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP2108_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2108Device::GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE lpbLength, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;
    int length;
    int index;

    // Validate parameter
    if (!ValidParam(lpManufacturer, lpbLength)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        index = devDesc.iManufacturer;
    }

    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpManufacturer, CP210x_MAX_MANUFACTURER_STRLEN);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        length = libusb_get_string_descriptor(m_handle, index, 0x0000, (unsigned char*) lpManufacturer, CP210x_MAX_MANUFACTURER_STRLEN);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    }

    return status;
}

CP210x_STATUS CCP2108Device::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    int length;
    int index;

    // Validate parameter
    if (!ValidParam(lpInterface, lpbLength)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bInterfaceNumber != 0x00) &&
            (bInterfaceNumber != 0x01) &&
            (bInterfaceNumber != 0x02) &&
            (bInterfaceNumber != 0x03)) {
        return CP210x_INVALID_PARAMETER;
    }

    // We will only be obtaining interface strings from CP2108 and they are in
    // string 3 4 5and 6, so we will simply add our interface index (0 or 1) to determine
    // which one we want from the interface
    index = 3 + bInterfaceNumber;

    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpInterface, CP2108_MAX_INTERFACE_STRLEN);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        length = libusb_get_string_descriptor(m_handle, index, 0x0000, (unsigned char*) lpInterface, CP2108_MAX_INTERFACE_STRLEN);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    }

    return status;
}

CP210x_STATUS CCP2108Device::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];

    // Validate parameter
    if (!ValidParam(lpwFlushBufferConfig)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370D, 0, setup, 2, 0) == 2) {
        *lpwFlushBufferConfig = setup[0] | (setup[1] << 8);
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2108Device::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::GetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    int transferSize = 73;

    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370C, 0, (BYTE*) QuadPortConfig, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2108Device::GetLockValue(LPBYTE lpbLockValue) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];

    // Validate parameter
    if (!ValidParam(lpbLockValue)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370A, 0, setup, 1, 0) == 1) {
        if (setup[0] == 0xFF)
            *lpbLockValue = 0x00;
        else
            *lpbLockValue = 0x01;
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2108Device::SetManufacturerString(LPVOID lpvManufacturer, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvManufacturer)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bLength > CP210x_MAX_MANUFACTURER_STRLEN) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    CopyToString(setup, lpvManufacturer, &length, bConvertToUnicode);

    transferSize = length + 2;
    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3714, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }
    
    return status;
}

CP210x_STATUS CCP2108Device::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvInterface)) {
        return CP210x_INVALID_PARAMETER;
    }


    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    if ((bLength > CP2108_MAX_INTERFACE_STRLEN) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (((bInterfaceNumber == 0x00) ||
            (bInterfaceNumber == 0x01) ||
            (bInterfaceNumber == 0x02) ||
            (bInterfaceNumber == 0x03))) {
        BYTE bSetupCmd = 0x00;
        // Copy string will alter the length if the string has to be converted to unicode.
        CopyToString(setup, lpvInterface, &length, bConvertToUnicode);

        if (bInterfaceNumber == 0x00) {
            bSetupCmd = 0x0F; // Set Interface 0 String command
        }
        if (bInterfaceNumber == 0x01) {
            bSetupCmd = 0x10; // Set Interface 1 String command
        }
        if (bInterfaceNumber == 0x02) {
            bSetupCmd = 0x12; // Set Interface 2 String command
        }
        if (bInterfaceNumber == 0x03) {
            bSetupCmd = 0x13; // Set Interface 3 String command
        }

        transferSize = length + 2;
        if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3700 | bSetupCmd, 0, setup, transferSize, 0) == transferSize) {
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        status = CP210x_INVALID_PARAMETER;
    }

    return status;
}

CP210x_STATUS CCP2108Device::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370D, wFlushBufferConfig, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2108Device::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::SetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2108Device::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    int transferSize = 73;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370C, 0, (BYTE*) QuadPortConfig, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2108Device::SetLockValue() {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370A, 0xF0, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}
