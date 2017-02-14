/////////////////////////////////////////////////////////////////////////////
// CP210xDevice.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "CP210xDevice.h"
#include "CP2101Device.h"
#include "CP2102Device.h"
#include "CP2103Device.h"
#include "CP2104Device.h"
#include "CP2105Device.h"
#include "CP2108Device.h"
#include "CP2109Device.h"
#include "CP210xSupportFunctions.h"

CCP210xDevice::~CCP210xDevice() {
}

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class - Static Methods
/////////////////////////////////////////////////////////////////////////////

CP210x_STATUS CCP210xDevice::GetNumDevices(LPDWORD lpdwNumDevices) {
    CP210x_STATUS status;

    size_t count;
    libusb_device** list;

    // Enumerate all USB devices, returning the number
    // of devices and a list of devices
    count = libusb_get_device_list(NULL, &list);

    // A negative count indicates an error
    if (count >= 0) {
        *lpdwNumDevices = count;
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_GLOBAL_DATA_ERROR;
    }

    // Unreference all devices to free the device list
    libusb_free_device_list(list, 1);

    return status;
}

CP210x_STATUS CCP210xDevice::Open(DWORD dwDevice, CCP210xDevice** devObj) {
    CP210x_STATUS status;

    size_t count;
    libusb_device** list;
    libusb_device_handle* h;
    BYTE partNum;

    // Enumerate all USB devices, returning the number
    // of devices and a list of devices
    count = libusb_get_device_list(NULL, &list);

    *devObj = NULL;

    // A negative count indicates an error
    if (count >= 0) {
        if (libusb_open(list[dwDevice], &h) == 0) {
            status = CCP210xDevice::GetDevicePartNumber(h, &partNum);
            if (status == CP210x_SUCCESS) {
                switch (partNum) {
                    case 1:
                        *devObj = (CCP210xDevice*)new CCP2101Device(h);
                        break;
                    case 2:
                        *devObj = (CCP210xDevice*)new CCP2102Device(h);
                        break;
                    case 3:
                        *devObj = (CCP210xDevice*)new CCP2103Device(h);
                        break;
                    case 4:
                        *devObj = (CCP210xDevice*)new CCP2104Device(h);
                        break;
                    case 5:
                        *devObj = (CCP210xDevice*)new CCP2105Device(h);
                        break;
                    case 8:
                        *devObj = (CCP210xDevice*)new CCP2108Device(h);
                        break;
                    case 9:
                        *devObj = (CCP210xDevice*)new CCP2109Device(h);
                        break;
                    default:
                        status = CP210x_DEVICE_NOT_FOUND;
                        break;
                }
            } else {
                status = CP210x_DEVICE_NOT_FOUND;
            }
        } else {
            status = CP210x_DEVICE_NOT_FOUND;
        }
    } else {
        status = CP210x_GLOBAL_DATA_ERROR;
    }

    // Unreference all devices to free the device list
    libusb_free_device_list(list, 1);

    return status;
}

CP210x_STATUS CCP210xDevice::GetDevicePartNumber(libusb_device_handle* h, LPBYTE lpbPartNum) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_config_descriptor* configDesc;

    // Validate parameter
    if (!ValidParam(lpbPartNum)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_control_transfer(h, 0xC0, 0xFF, 0x370B, 0x0000, lpbPartNum, 1, 0) == 1) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
        if (libusb_get_config_descriptor(libusb_get_device(h), 0, &configDesc) == 0) {
            if (((configDesc->interface[0].altsetting->endpoint[0].bEndpointAddress & 0x0F) == 0x03) &&
                    ((configDesc->interface[0].altsetting->endpoint[1].bEndpointAddress & 0x0F) == 0x03)) {
                *lpbPartNum = CP210x_CP2101_VERSION;
                status = CP210x_SUCCESS;
            }
            libusb_free_config_descriptor(configDesc);
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class - Public Methods
/////////////////////////////////////////////////////////////////////////////

CP210x_STATUS CCP210xDevice::Reset() {
    libusb_reset_device(m_handle);
    return CP210x_SUCCESS;
}

CP210x_STATUS CCP210xDevice::Close() {
    libusb_close(m_handle);
    m_handle = NULL;
    return CP210x_SUCCESS;
}

HANDLE CCP210xDevice::GetHandle() {
    return this;
}

CP210x_STATUS CCP210xDevice::GetPartNumber(LPBYTE lpbPartNum) {
    // Validate parameter
    if (!ValidParam(lpbPartNum)) {
        return CP210x_INVALID_PARAMETER;
    }

    *lpbPartNum = m_partNumber;

    return CP210x_SUCCESS;
}

CP210x_STATUS CCP210xDevice::SetVid(WORD wVid) {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3701, wVid, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetPid(WORD wPid) {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3702, wPid, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetProductString(LPVOID lpvProduct, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE str[CP210x_MAX_DEVICE_STRLEN];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvProduct)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bLength > maxProductStrLen) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    CopyToString(str, lpvProduct, &length, bConvertToUnicode);

    transferSize = length + 2;
    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3703, 0, str, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }
    
    return status;
}

CP210x_STATUS CCP210xDevice::SetSerialNumber(LPVOID lpvSerialNumber, BYTE bLength, BOOL bConvertToUnicode) {
    CP210x_STATUS status;
    BYTE str[CP210x_MAX_DEVICE_STRLEN];
    BYTE length = bLength;
    int transferSize;

    // Validate parameter
    if (!ValidParam(lpvSerialNumber)) {
        return CP210x_INVALID_PARAMETER;
    }

    if ((bLength > maxProductStrLen) || (bLength < 1)) {
        return CP210x_INVALID_PARAMETER;
    }

    CopyToString(str, lpvSerialNumber, &length, bConvertToUnicode);

    transferSize = length + 2;
    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3704, 0, str, transferSize, 0) == transferSize) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }
    
    return status;
}

CP210x_STATUS CCP210xDevice::SetSelfPower(BOOL bSelfPower) {
    CP210x_STATUS status;
    BYTE bPowerAttrib = 0x80;

    if (bSelfPower)
        bPowerAttrib |= 0x40; // Set the self-powered bit.

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3705, bPowerAttrib, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetMaxPower(BYTE bMaxPower) {
    CP210x_STATUS status;
    if (bMaxPower > CP210x_MAX_MAXPOWER) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3706, bMaxPower, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::SetDeviceVersion(WORD wVersion) {
    CP210x_STATUS status;

    if (libusb_control_transfer(m_handle, 0x40, 0xFF, 0x3707, wVersion, NULL, 0, 0) == 0) {
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetVid(LPWORD lpwVid) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(lpwVid)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *lpwVid = devDesc.idVendor;

        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetPid(LPWORD lpwPid) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(lpwPid)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *lpwPid = devDesc.idProduct;

        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceProductString(LPVOID lpProduct, LPBYTE lpbLength, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    //libusb_device_descriptor devDesc;
    int length;
    int index;

    // Validate parameter
    if (!ValidParam(lpProduct, lpbLength)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    //if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
    //    index = devDesc.idProduct;
    //}
    // We need to hard code the product string to 2, because CP2105 declares the interface 0
    // string in iProduct
    index = 2;

    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpProduct, maxProductStrLen);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        length = libusb_get_string_descriptor(m_handle, index, 0x0000, (unsigned char*) lpProduct, maxProductStrLen);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceSerialNumber(LPVOID lpSerial, LPBYTE lpbLength, BOOL bConvertToASCII) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;
    int length;
    int index;

    // Validate parameter
    if (!ValidParam(lpSerial, lpbLength)) {
        return CP210x_INVALID_PARAMETER;
    }

    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        index = devDesc.iSerialNumber;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    // We need to hard code the product string to 2, because CP2105 declares the interface 0
    // string in iProduct
    if (bConvertToASCII) {
        length = libusb_get_string_descriptor_ascii(m_handle, index, (unsigned char*) lpSerial, maxSerialStrLen);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    } else {
        length = libusb_get_string_descriptor(m_handle, index, 0x0000, (unsigned char*) lpSerial, maxSerialStrLen);
        if (length > 0) {
            *lpbLength = (BYTE) (length & 0xFF);
            status = CP210x_SUCCESS;
        } else {
            status = CP210x_DEVICE_IO_FAILED;
        }
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetSelfPower(LPBOOL lpbSelfPower) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_config_descriptor* configDesc;

    // Validate parameter
    if (!ValidParam(lpbSelfPower)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_config_descriptor(libusb_get_device(m_handle), 0, &configDesc) == 0) {
        if (configDesc->bmAttributes & 0x40)
            *lpbSelfPower = TRUE;
        else
            *lpbSelfPower = FALSE;
        libusb_free_config_descriptor(configDesc);
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetMaxPower(LPBYTE lpbMaxPower) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_config_descriptor* configDesc;

    // Validate parameter
    if (!ValidParam(lpbMaxPower)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_config_descriptor(libusb_get_device(m_handle), 0, &configDesc) == 0) {
        *lpbMaxPower = configDesc->MaxPower;
        libusb_free_config_descriptor(configDesc);
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}

CP210x_STATUS CCP210xDevice::GetDeviceVersion(LPWORD lpwVersion) {
    CP210x_STATUS status = CP210x_INVALID_HANDLE;
    libusb_device_descriptor devDesc;

    // Validate parameter
    if (!ValidParam(lpwVersion)) {
        return CP210x_INVALID_PARAMETER;
    }

    // Get descriptor that contains the index of the USB_STRING_DESCRIPTOR containing the Product String
    if (libusb_get_device_descriptor(libusb_get_device(m_handle), &devDesc) == 0) {
        *lpwVersion = devDesc.bcdDevice;
        status = CP210x_SUCCESS;
    } else {
        status = CP210x_DEVICE_IO_FAILED;
    }

    return status;
}



