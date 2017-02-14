/*
 * CP2105Device.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2105Device.h"
#include "CP210xSupportFunctions.h"

CCP2105Device::CCP2105Device(libusb_device_handle* h) {
    m_handle = h;
    m_partNumber = 0x05;
    maxSerialStrLen = CP2105_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP2105_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2105Device::GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE lpbLength, BOOL bConvertToASCII) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    int length;
    int index;

    // Validate parameter
    if (!ValidParam(lpInterface, lpbLength)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bInterfaceNumber != 0x00) &&
            (bInterfaceNumber != 0x01)) {
        return CP210x_INVALID_PARAMETER;
    }

    // We will only be obtaining interface strings from CP2105 and they are in
    // string 3 and 4, so we will simply add our interface index (0 or 1) to determine
    // which one we want from the interface
    index = 3 + bInterfaceNumber;

    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpInterface, CP2105_MAX_INTERFACE_STRLEN);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        length = libusb_get_string_descriptor(m_handle, index, 0x0000, (unsigned char*) lpInterface, CP2105_MAX_INTERFACE_STRLEN);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    }

    return status;
}

CP210x_STATUS CCP2105Device::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];

    // Validate parameter
    if (!ValidParam(lpwFlushBufferConfig)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370D, 0, setup, 1, 0) == 1) {
        *lpwFlushBufferConfig = setup[0];
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2105Device::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    int transferSize = 2;

    // Validate parameter
    if (!ValidParam(lpbDeviceModeECI)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Validate parameter
    if (!ValidParam(lpbDeviceModeSCI)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x3711, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
        *lpbDeviceModeECI = setup[0];
        *lpbDeviceModeSCI = setup[1];
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;

}

CP210x_STATUS CCP2105Device::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::GetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    int transferSize = 15;

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370C, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
        DualPortConfig->Mode = (setup[0] << 8) + setup[1];
        //PortConfig->Reset.LowPower = (setup[10] << 8) + setup[11];
        DualPortConfig->Reset_Latch = (setup[4] << 8) + setup[5];
        //PortConfig->Suspend.Mode = (setup[14] << 8) + setup[15];
        //PortConfig->Suspend.LowPower = (setup[16] << 8) + setup[17];
        DualPortConfig->Suspend_Latch = (setup[10] << 8) + setup[11];
        DualPortConfig->EnhancedFxn_SCI = setup[12];
        DualPortConfig->EnhancedFxn_ECI = setup[13];
        DualPortConfig->EnhancedFxn_Device = setup[14];

        // Mask out reserved bits in EnhancedFxn
        //DualPortConfig->EnhancedFxnECI &= ~EF_DYNAMIC_SUSPEND_ECI;
        //DualPortConfig->EnhancedFxnSCI &= ~EF_DYNAMIC_SUSPEND_SCI;
        DualPortConfig->EnhancedFxn_Device &= ~EF_RESERVED_1;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2105Device::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::GetLockValue(LPBYTE lpbLockValue) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];

    // Validate parameter
    if (!ValidParam(lpbLockValue)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

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

CP210x_STATUS CCP2105Device::SetManufacturerString(LPVOID lpvManufacturer, BYTE bLength, BOOL bConvertToUnicode) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvInterface)) {
        return CP210x_INVALID_PARAMETER;
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    if ((bLength > CP2105_MAX_INTERFACE_STRLEN) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (((bInterfaceNumber == 0x00) ||
            (bInterfaceNumber == 0x01))) {
        BYTE bSetupCmd = 0x00;
        // Copy string will alter the length if the string has to be converted to unicode.
        CopyToString(setup, lpvInterface, &length, bConvertToUnicode);

        if (bInterfaceNumber == 0x00) {
            bSetupCmd = 0x0F; // Set Interface 0 String command
        }
        if (bInterfaceNumber == 0x01) {
            bSetupCmd = 0x10; // Set Interface 1 String command
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

CP210x_STATUS CCP2105Device::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370D, wFlushBufferConfig, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2105Device::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    int transferSize = 4;

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    setup[0] = bDeviceModeECI;
    setup[1] = bDeviceModeSCI;
    setup[2] = 0;
    setup[3] = 0;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3711, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2105Device::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::SetPortConfig(PORT_CONFIG* PortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE Temp_EnhancedFxn_ECI, Temp_EnhancedFxn_SCI, Temp_EnhancedFxn_Device;
    int transferSize = 15;

    // Change user Port_Config structure to match firmware, and check reserved bits are zero
    if ((DualPortConfig->EnhancedFxn_Device & EF_RESERVED_1) /*||
		(DualPortConfig->EnhancedFxn_ECI & EF_DYNAMIC_SUSPEND_ECI) ||
		(DualPortConfig->EnhancedFxn_SCI & EF_DYNAMIC_SUSPEND_SCI)*/)
        return CP210x_INVALID_PARAMETER;

    Temp_EnhancedFxn_ECI = DualPortConfig->EnhancedFxn_ECI; // save user settings into temp variable to send out
    Temp_EnhancedFxn_SCI = DualPortConfig->EnhancedFxn_SCI;
    Temp_EnhancedFxn_Device = DualPortConfig->EnhancedFxn_Device;

    if (Temp_EnhancedFxn_Device & EF_WEAKPULLUP) {
        Temp_EnhancedFxn_Device |= 0x30; // Set both Weak Pullup bits
    } else {
        Temp_EnhancedFxn_Device &= ~0x30; // Clear both Weak Pullup bits
    }

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    setup[0] = (DualPortConfig->Mode & 0xFF00) >> 8;
    setup[1] = (DualPortConfig->Mode & 0x00FF);
    setup[2] = 0x00;
    setup[3] = 0x00;
    setup[4] = (DualPortConfig->Reset_Latch & 0xFF00) >> 8;
    setup[5] = (DualPortConfig->Reset_Latch & 0x00FF);
    setup[6] = (DualPortConfig->Mode & 0xFF00) >> 8;
    setup[7] = (DualPortConfig->Mode & 0x00FF);
    setup[8] = 0x00;
    setup[9] = 0x00;
    setup[10] = (DualPortConfig->Suspend_Latch & 0xFF00) >> 8;
    setup[11] = (DualPortConfig->Suspend_Latch & 0x00FF);
    setup[12] = Temp_EnhancedFxn_SCI;
    setup[13] = Temp_EnhancedFxn_ECI;
    setup[14] = Temp_EnhancedFxn_Device;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370C, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2105Device::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2105Device::SetLockValue() {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370A, 0xF0, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}
