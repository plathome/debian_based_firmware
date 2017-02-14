/*
 * CP2103Device.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: strowlan
 */

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP2103Device.h"
#include "CP210xSupportFunctions.h"

CCP2103Device::CCP2103Device(libusb_device_handle* h) {
    m_handle = h;
    m_partNumber = 0x03;
    maxSerialStrLen = CP210x_MAX_SERIAL_STRLEN;
    maxProductStrLen = CP210x_MAX_PRODUCT_STRLEN;
}

CP210x_STATUS CCP2103Device::GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE lpbLength, BOOL bConvertToASCII) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::GetFlushBufferConfig(LPWORD lpwFlushBufferConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::GetDeviceMode(LPBYTE lpbDeviceModeECI, LPBYTE lpbDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::GetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    int transferSize = (NUM_BAUD_CONFIGS * BAUD_CONFIG_SIZE);

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x3709, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
        BAUD_CONFIG* currentBaudConfig;
        currentBaudConfig = baudConfigData;

        for (int i = 0; i < transferSize; i += BAUD_CONFIG_SIZE) {
            currentBaudConfig->BaudGen = (setup[i] << 8) + setup[i + 1];
            currentBaudConfig->Timer0Reload = (setup[i + 2] << 8) + setup[i + 3];
            currentBaudConfig->Prescaler = setup[i + 4];
            //setup[i+5] reserved for later use
            currentBaudConfig->BaudRate = setup[i + 6] + (setup[i + 7] << 8) + (setup[i + 8] << 16) + (setup[i + 9] << 24);

            currentBaudConfig++;
        }
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2103Device::GetPortConfig(PORT_CONFIG* PortConfig) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    int transferSize = 13;

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    if (libusb_control_transfer(m_handle, 0xC0, 0xFF, 0x370C, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
        PortConfig->Mode = (setup[0] << 8) + setup[1];
        //PortConfig->Reset.LowPower = (setup[10] << 8) + setup[11];
        PortConfig->Reset_Latch = (setup[4] << 8) + setup[5];
        //PortConfig->Suspend.Mode = (setup[14] << 8) + setup[15];
        //PortConfig->Suspend.LowPower = (setup[16] << 8) + setup[17];
        PortConfig->Suspend_Latch = (setup[10] << 8) + setup[11];
        PortConfig->EnhancedFxn = setup[12];

        // Mask out reserved bits in EnhancedFxn
        PortConfig->EnhancedFxn &= ~(EF_SERIAL_DYNAMIC_SUSPEND | EF_RESERVED_1);
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2103Device::GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::GetLockValue(LPBYTE lpbLockValue) {
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

CP210x_STATUS CCP2103Device::SetManufacturerString(LPVOID lpvManufacturer, BYTE bLength, BOOL bConvertToUnicode) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::SetFlushBufferConfig(WORD wFlushBufferConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::SetBaudRateConfig(BAUD_CONFIG* baudConfigData) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BAUD_CONFIG* currentBaudConfig;
    int transferSize = (NUM_BAUD_CONFIGS * BAUD_CONFIG_SIZE);
    currentBaudConfig = baudConfigData;

    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);
    
    for (int i = 0; i < transferSize; i += BAUD_CONFIG_SIZE) {
        setup[i] = (currentBaudConfig->BaudGen & 0xFF00) >> 8;
        setup[i + 1] = currentBaudConfig->BaudGen & 0x00FF;
        setup[i + 2] = (currentBaudConfig->Timer0Reload & 0xFF00) >> 8;
        setup[i + 3] = currentBaudConfig->Timer0Reload & 0x00FF;
        setup[i + 4] = currentBaudConfig->Prescaler;
        setup[i + 5] = 0x00; //reserved for later
        setup[i + 6] = (BYTE) (currentBaudConfig->BaudRate & 0x000000FF);
        setup[i + 7] = (BYTE) ((currentBaudConfig->BaudRate & 0x0000FF00) >> 8);
        setup[i + 8] = (BYTE) ((currentBaudConfig->BaudRate & 0x00FF0000) >> 16);
        setup[i + 9] = (BYTE) ((currentBaudConfig->BaudRate & 0xFF000000) >> 24);

        currentBaudConfig++;
    }

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3709, 0, setup, transferSize + 2, 0) == transferSize + 2) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2103Device::SetPortConfig(PORT_CONFIG* PortConfig) {
    CP210x_STATUS status;
    BYTE setup[CP210x_MAX_SETUP_LENGTH];
    BYTE Temp_EnhancedFxn;
    int transferSize = 13;

    // Change user Port_Config structure to match firmware, and check reserved bits are zero
    if (PortConfig->EnhancedFxn & (EF_SERIAL_DYNAMIC_SUSPEND | EF_RESERVED_1))
        return CP210x_INVALID_PARAMETER;

    Temp_EnhancedFxn = PortConfig->EnhancedFxn; // save user settings into temp variable to send out

    if (Temp_EnhancedFxn & EF_WEAKPULLUP) {
        Temp_EnhancedFxn |= 0x30; // Set both Weak Pullup bits
    } else {
        Temp_EnhancedFxn &= ~0x30; // Clear both Weak Pullup bits
    }
    
    memset(setup, 0, CP210x_MAX_SETUP_LENGTH);

    setup[0] = (PortConfig->Mode & 0xFF00) >> 8;
    setup[1] = (PortConfig->Mode & 0x00FF);
    setup[2] = 0x00; //(PortConfig->Reset.LowPower & 0xFF00) >> 8;
    setup[3] = 0x00; //(PortConfig->Reset.LowPower & 0x00FF);
    setup[4] = (PortConfig->Reset_Latch & 0xFF00) >> 8;
    setup[5] = (PortConfig->Reset_Latch & 0x00FF);
    setup[6] = (PortConfig->Mode & 0xFF00) >> 8;
    setup[7] = (PortConfig->Mode & 0x00FF);
    setup[8] = 0x00; //(PortConfig->Suspend.LowPower & 0xFF00) >> 8;
    setup[9] = 0x00; //(PortConfig->Suspend.LowPower & 0x00FF);
    setup[10] = (PortConfig->Suspend_Latch & 0xFF00) >> 8;
    setup[11] = (PortConfig->Suspend_Latch & 0x00FF);
    setup[12] = Temp_EnhancedFxn;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370C, 0, setup, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP2103Device::SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig) {
    return CP210x_FUNCTION_NOT_SUPPORTED;
}

CP210x_STATUS CCP2103Device::SetLockValue() {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x370A, 0xF0, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}
