/////////////////////////////////////////////////////////////////////////////
// CP210xDevice.h
/////////////////////////////////////////////////////////////////////////////

#ifndef CP210x_DEVICE_H
#define CP210x_DEVICE_H

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "libusb.h"
#include "CP210xManufacturing.h"

/////////////////////////////////////////////////////////////////////////////
// CCP210xDevice Class
/////////////////////////////////////////////////////////////////////////////

class CCP210xDevice
{
// Static Methods
public:
    static CP210x_STATUS GetNumDevices(LPDWORD lpdwNumDevices);
    static CP210x_STATUS Open(DWORD dwDevice, CCP210xDevice** devObj);

    virtual ~CCP210xDevice();

private:
    static CP210x_STATUS GetDevicePartNumber(libusb_device_handle* h, LPBYTE lpbPartNum);
    
// Public Methods
public:
    CP210x_STATUS Reset();
    CP210x_STATUS Close();
    HANDLE GetHandle();

    CP210x_STATUS GetPartNumber(LPBYTE lpbPartNum);
    
    CP210x_STATUS SetVid(WORD wVid);
    CP210x_STATUS SetPid(WORD wPid);
    CP210x_STATUS SetProductString(LPVOID lpvProduct, BYTE bLength, BOOL bConvertToUnicode = true);
    CP210x_STATUS SetSerialNumber(LPVOID lpvSerialNumber, BYTE bLength,	BOOL bConvertToUnicode = true);
    CP210x_STATUS SetSelfPower(BOOL bSelfPower);
    CP210x_STATUS SetMaxPower(BYTE bMaxPower);
    CP210x_STATUS SetDeviceVersion(WORD wVersion);

    CP210x_STATUS GetVid(LPWORD wVid);
    CP210x_STATUS GetPid(LPWORD wPid);
    CP210x_STATUS GetProductString(DWORD dwDeviceNum, LPVOID lpvDeviceString, DWORD dwFlags);
    CP210x_STATUS GetDeviceProductString(LPVOID lpProduct, LPBYTE lpbLength, BOOL bConvertToASCII = true);
    CP210x_STATUS GetDeviceSerialNumber(LPVOID lpSerial, LPBYTE lpbLength, BOOL bConvertToASCII = true);
    CP210x_STATUS GetSelfPower(LPBOOL lpbSelfPower);
    CP210x_STATUS GetMaxPower(LPBYTE lpbMaxPower);
    CP210x_STATUS GetDeviceVersion(LPWORD lpwVersion);


    // virtual functions
    virtual CP210x_STATUS GetDeviceManufacturerString(LPVOID lpManufacturer, LPBYTE lpbLength, BOOL bConvertToASCII = true)=0;
    virtual CP210x_STATUS GetDeviceInterfaceString(BYTE bInterfaceNumber, LPVOID lpInterface, LPBYTE lpbLength, BOOL bConvertToASCII)=0;
    virtual CP210x_STATUS GetFlushBufferConfig(LPWORD lpwFlushBufferConfig)=0;
    virtual CP210x_STATUS GetDeviceMode(LPBYTE lpbDeviceModeECI,LPBYTE lpbDeviceModeSCI)=0;
    virtual CP210x_STATUS GetBaudRateConfig(BAUD_CONFIG* baudConfigData)=0;
    virtual CP210x_STATUS GetPortConfig(PORT_CONFIG* PortConfig)=0;
    virtual CP210x_STATUS GetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig)=0;
    virtual CP210x_STATUS GetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig)=0;
    virtual CP210x_STATUS GetLockValue(LPBYTE lpbLockValue)=0;


    virtual CP210x_STATUS SetManufacturerString(LPVOID lpvManufacturer, BYTE bLength, BOOL bConvertToUnicode = true)=0;
    virtual CP210x_STATUS SetInterfaceString(BYTE bInterfaceNumber, LPVOID lpvInterface, BYTE bLength, BOOL bConvertToUnicode)=0;
    virtual CP210x_STATUS SetFlushBufferConfig(WORD wFlushBufferConfig)=0;
    virtual CP210x_STATUS SetDeviceMode(BYTE bDeviceModeECI, BYTE bDeviceModeSCI)=0;
    virtual CP210x_STATUS SetBaudRateConfig(BAUD_CONFIG* baudConfigData)=0;
    virtual CP210x_STATUS SetPortConfig(PORT_CONFIG* PortConfig)=0;
    virtual CP210x_STATUS SetDualPortConfig(DUAL_PORT_CONFIG* DualPortConfig)=0;
    virtual CP210x_STATUS SetQuadPortConfig(QUAD_PORT_CONFIG* QuadPortConfig)=0;
    virtual CP210x_STATUS SetLockValue()=0;

// Protected Members
protected:
    libusb_device_handle* m_handle;
    BYTE m_partNumber;
    
    BYTE maxSerialStrLen;
    BYTE maxProductStrLen;
};

#endif // CP210x_DEVICE_H
