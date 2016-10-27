//-----------------------------------------------------------------------------
// SLAB_USB_SPI.h
//-----------------------------------------------------------------------------
// Copyright 2012-2013 Silicon Laboratories, Inc.
// http://www.silabs.com
//-----------------------------------------------------------------------------

/// @file SLAB_USB_SPI.h
/// This file defines the API of the CP2130 Interface Library

#ifndef SLAB_USB_SPI_H
#define SLAB_USB_SPI_H

/////////////////////////////////////////////////////////////////////////////
// Type Definitions - Windows Types
/////////////////////////////////////////////////////////////////////////////
/// @name Windows Type Definitions
/// @{

typedef void*               HANDLE;
typedef void*               LPVOID;
typedef int                 BOOL;
typedef BOOL*               LPBOOL;
typedef unsigned char       BYTE;
typedef BYTE                UCHAR;
typedef BYTE*               LPBYTE;
typedef UCHAR*              PUCHAR;
typedef unsigned short      WORD;
typedef WORD*               LPWORD;
typedef unsigned long       DWORD;
typedef DWORD               ULONG;
typedef DWORD*              LPDWORD;
typedef ULONG*              PULONG;
typedef char*               LPSTR;
typedef const char*         LPCSTR;
#define CP213x_MAX_DEVICE_STRLEN 256
typedef		char	CP213x_DEVICE_STRING[CP213x_MAX_DEVICE_STRLEN];

#define TRUE 1
#define FALSE 0
#define INVALID_HANDLE_VALUE -1;
/// @}

/////////////////////////////////////////////////////////////////////////////
// Definitions - Windows Definitions
/////////////////////////////////////////////////////////////////////////////
/// @name Windows Definitions
/// @{

#define MAX_PATH            260
#define CALLBACK            __stdcall           
/// @}

/////////////////////////////////////////////////////////////////////////////
// Variable types
/////////////////////////////////////////////////////////////////////////////
/// @name Variable type definitions
/// @{
typedef void*                       CP213x_DEVICE;
typedef int                         USB_SPI_STATUS;
/// @}

/////////////////////////////////////////////////////////////////////////////
// Definitions specific to USB SPI code
/////////////////////////////////////////////////////////////////////////////
/// @name Definitions specific to USB SPI code
/// @{

//#define LIBVERSION_MAJOR        1
//#define LIBVERSION_MINOR        0
//#define LIBVERSION_ISRELEASE    true
#define LANGIDENG               0x0409                  



#define BULK_PACKETSIZE             64
#define EP_BUFSIZE                  BULK_PACKETSIZE
#define CMD_SIZE                    8
#define CMD_TIMEOUT_MS              1000        // Timeout for completion of SPI command (i.e. first packet)
#define STRING_DESCRIPTOR_SIZE      256
#define SIZE_PIN_CONFIG             20


// Maximum block size for WriteRead
#define WRITEREAD_MAX_BLOCKSIZE     256         // Matches size of firmware Read buffer

// Number of GPIO/CS pins
#define    CP213x_NUM_GPIO          11

// Chip Select Mode:  0: Idle, 1: Active, 2: Active; all other channels idle
#define CSMODE_IDLE                 0
#define CSMODE_ACTIVE               1
#define CSMODE_ACTIVE_OTHERS_IDLE   2

// Mode definitions for GPIO/CS/SpecialFunction pins
#define GPIO_MODE_INPUT                 0
#define GPIO_MODE_OUTPUT_OD             1
#define GPIO_MODE_OUTPUT_PP             2
#define GPIO_MODE_CHIPSELECT            3       // Chip select mode

#define GPIO_MODE__RTR_ACTLOW           4
#define GPIO_MODE__RTR_ACTHIGH          5

#define GPIO_MODE__EVTCNTR_RISING       4
#define GPIO_MODE__EVTCNTR_FALLING      5
#define GPIO_MODE__EVTCNTR_NEGPULSE     6
#define GPIO_MODE__EVTCNTR_POSPULSE     7

#define GPIO_MODE__CLK_OUT              4
#define GPIO_MODE__SPI_ACTIVITY         4
#define GPIO_MODE__SUSPEND              4
#define GPIO_MODE__SUSPENDBAR           4

//  End of GPIO Mode definitions


// String Buffer Definitions
#define SHORT_STRING_LEN            60
#define LONG_STRING_LEN_1           61
#define LONG_STRING_LEN_2           63
/// @}

////////////////////////////////////////////////////////////////////////////////
// SPI Command and Subcommand definitions
////////////////////////////////////////////////////////////////////////////////

/// @name SPI Data Transfer command and subcommands
/// @{
#define CMD_TRANSFER_DATA           0x0000
    // Subcommand definitions for TRANSFER_DATA command
    #define SUBCMD_READ             0
    #define SUBCMD_WRITE            1
    #define SUBCMD_WRITEREAD        2
    #define SUBCMD_RTREAD           4
    #define SUBCMD_MSB_RELEASE_BUS  0x80
/// @}

/// @name SPI Control Word bitfield definitions
/// @{
// Bits 7-6: Not assigned

// Bit 5: Clock phase (CPHA)
#define SPICTL_CPHA_SHIFT           5
#define SPICTL_CPHA_MASK            0x20
#define SPICTL_CPHA_LEADING_EDGE    0
#define SPICTL_CPHA_TRAILING_EDGE   1

// Bit 4: Clock polarity (CPOL)
#define SPICTL_CPOL_SHIFT           4
#define SPICTL_CPOL_MASK            0x10
#define SPICTL_CPOL_ACTIVE_HIGH     0
#define SPICTL_CPOL_ACTIVE_LOW      1

// Bit 3: Chip Select Mode
#define SPICTL_CSMODE_SHIFT         3
#define SPICTL_CSMODE_MASK          0x08
#define SPICTL_CSMODE_OPENDRAIN     0
#define SPICTL_CSMODE_PUSHPULL      1

// Bit 2-0: Clock rate
#define SPICTL_CLKRATE_SHIFT        0
#define SPICTL_CLKRATE_MASK         0x07
#define SPICTL_CLKRATE_12M          0       // 12 MHz
#define SPICTL_CLKRATE_6M           1       // 6 MHz
#define SPICTL_CLKRATE_3M           2       // 3 MHz
#define SPICTL_CLKRATE_1M5          3       // 1.5 MHz
#define SPICTL_CLKRATE_750K         4       // 750 kHz
#define SPICTL_CLKRATE_375K         5       // 375 kHz
#define SPICTL_CLKRATE_187K5        6       // 187.5 kHz
#define SPICTL_CLKRATE_93K75        7       // 93.75 kHz
/// @}

/////////////////////////////////////////////////////////////////////////////
// Error Definitions
/////////////////////////////////////////////////////////////////////////////

/// @name Return values for BOOL-returning functions
/// @{
#define bRESULT_PASS    TRUE
#define bRESULT_FAIL    FALSE
/// @}

#define USB_SPI_ERRCODE_SYSTEM_ERROR                    0xFFFFFFFE

/// @name CP2130 General Errors
/// @{

#define USB_SPI_ERRCODE_SUCCESS                         0x00    /// The function returned successfully.
#define USB_SPI_ERRCODE_ALLOC_FAILURE                   0x01    /// Allocation error.
#define USB_SPI_ERRCODE_INVALID_ENUM_VALUE              0x02    /// Invalid enumeration value.
#define USB_SPI_ERRCODE_NULL_POINTER                    0x03    /// Null pointer.
#define USB_SPI_ERRCODE_INVALID_CHANNEL_INDEX           0x04    /// Channel index is not in range.
#define USB_SPI_ERRCODE_INVALID_GPIO_MODE               0x05    /// Mode != INPUT, OUTPUT_OD, or OUTPUT_PP.

#define USB_SPI_ERRCODE_UNKNOWN_ERROR                   0xFFFFFFFF  /// Unknown/unspecified error.
/// @}

//
// API Errors
//
/// @name CP2130 API Errors
/// @{
#define USB_SPI_ERRCODE_INVALID_PARAMETER               0x10    /// Invalid function parameter.

#define USB_SPI_ERRCODE_INVALID_DEVICE_OBJECT           0x11    /// The specified device object pointer is invalid.
/// @}

//
// Device Errors
//
/// @name CP2130 Device Errors
/// @{
// The specified USB device could not be found.
#define USB_SPI_ERRCODE_DEVICE_NOT_FOUND                0x20

// The current USB device is not opened.
#define USB_SPI_ERRCODE_USB_DEVICE_NOT_OPENED           0x21

// The handle is invalid.
#define USB_SPI_ERRCODE_INVALID_HANDLE                  0x22
/// @}

//
// Device Hardware Interface Errors
//
/// @name CP2130 Device Hardware Interface Errors
/// @{
// An error occurred while communicating with the device or while retrieving device information.
#define USB_SPI_ERRCODE_HWIF_DEVICE_ERROR               0x30

// A control transfer operation timed out.
#define USB_SPI_ERRCODE_HWIF_TRANSFER_TIMEOUT           0x31
/// @}

/// @name CP2130 Data Transfer Errors
/// @{

#define USB_SPI_ERRCODE_CONTROL_TRANSFER_ERROR          0x50
#define USB_SPI_ERRCODE_INVALID_TRANSFER_SIZE           0x51

#define USB_SPI_ERRCODE_PIPE_INIT_FAIL                  0x60
#define USB_SPI_ERRCODE_PIPE_QUERY_FAIL                 0x61
#define USB_SPI_ERRCODE_PIPE_WRITE_FAIL                 0x62
#define USB_SPI_ERRCODE_PIPE_READ_FAIL                  0x63
#define USB_SPI_ERRCODE_PIPE_ABORT_FAIL                 0x64
#define USB_SPI_ERRCODE_PIPE_FLUSH_FAIL                 0x65
#define USB_SPI_ERRCODE_PIPE_INVALID_ID                 0x66

#define USB_SPI_ERRCODE_READ_THREAD_CREATE_FAILURE      0x70
#define USB_SPI_ERRCODE_READ_THREAD_NOT_RUNNING         0x71
#define USB_SPI_ERRCODE_READ_THREAD_START_FAILURE       0x72

#define USB_SPI_ERRCODE_DEVICE_RETURNED_TOO_MUCH_DATA   0x80
/// @}

/// @name CP213x Customization Masks
/// @{
// User-Customizable Field Lock Bitmasks
#define CP213x_LOCK_PRODUCT_STR_1           0x0001
#define CP213x_LOCK_PRODUCT_STR_2           0x0002
#define CP213x_LOCK_SERIAL_STR              0x0004
#define CP213x_LOCK_PIN_CONFIG              0x0008

#define CP213x_LOCK_VID                     0x0100
#define CP213x_LOCK_PID                     0x0200
#define CP213x_LOCK_POWER                   0x0400
#define CP213x_LOCK_POWER_MODE              0x0800
#define CP213x_LOCK_RELEASE_VERSION         0x1000
#define CP213x_LOCK_MFG_STR_1               0x2000
#define CP213x_LOCK_MFG_STR_2               0x4000
#define CP213x_LOCK_TRANSFER_PRIORITY       0x8000

// USB Config Bitmasks
#define CP213x_SET_VID                      0x01
#define CP213x_SET_PID                      0x02
#define CP213x_SET_POWER                    0x04
#define CP213x_SET_POWER_MODE               0x08
#define CP213x_SET_RELEASE_VERSION          0x10
#define CP213x_SET_TRANSFER_PRIORITY        0x80

// String Lengths
#define MFG_STRLEN          62
#define PRODUCT_STRLEN      62
#define SERIAL_STRLEN       30
// MFG_STR
typedef char MFG_STR[MFG_STRLEN];
// PRODUCT_STR
typedef char PRODUCT_STR[PRODUCT_STRLEN];
// SERIAL_STR
typedef char SERIAL_STR[SERIAL_STRLEN];

/////////////////////////////////////////////////////////////////////////////
// Enumerations
/////////////////////////////////////////////////////////////////////////////

enum SET_STATUS {SET_SUCCESS, SET_FAIL, SET_VERIFY_FAIL};

/////////////////////////////////////////////////////////////////////////////
// Pin Definitions
/////////////////////////////////////////////////////////////////////////////

// Pin Config Mode Array Indices
#define CP213x_INDEX_GPIO_0         0
#define CP213x_INDEX_GPIO_1         1
#define CP213x_INDEX_GPIO_2         2
#define CP213x_INDEX_GPIO_3         3
#define CP213x_INDEX_GPIO_4         4
#define CP213x_INDEX_GPIO_5         5
#define CP213x_INDEX_GPIO_6         6
#define CP213x_INDEX_GPIO_7         7
#define CP213x_INDEX_GPIO_8         8
#define CP213x_INDEX_GPIO_9         9
#define CP213x_INDEX_GPIO_10        10

// Pin Bitmasks
#define CP213x_MASK_SCK             0x0001
#define CP213x_MASK_MISO            0x0002
#define CP213x_MASK_MOSI            0x0004
#define CP213x_MASK_GPIO_0          0x0008
#define CP213x_MASK_GPIO_1          0x0010
#define CP213x_MASK_GPIO_2          0x0020
#define CP213x_MASK_GPIO_3          0x0040
#define CP213x_MASK_GPIO_4          0x0080
#define CP213x_MASK_GPIO_5          0x0100
#define CP213x_MASK_GPIO_6          0x0400
#define CP213x_MASK_GPIO_7          0x0800
#define CP213x_MASK_GPIO_8          0x1000
#define CP213x_MASK_GPIO_9          0x2000
#define CP213x_MASK_GPIO_10         0x4000

/// @}

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

/// @name Type and structure definitions
/// @{

/// This struct has the same format as _USB_DEVICE_DESCRIPTOR, as defined in Windows usb100.h.
/// Using this typedef enables portable application code.
typedef struct DEVICE_DESCRIPTOR
{
    BYTE    bLength;
    BYTE    bDescriptorType;
    WORD    bcdUSB;
    BYTE    bDeviceClass;
    BYTE    bDeviceSubClass;
    BYTE    bDeviceProtocol;
    BYTE    bMaxPacketSize0;
    WORD    idVendor;
    WORD    idProduct;
    WORD    bcdDevice;
    BYTE    iManufacturer;
    BYTE    iProduct;
    BYTE    iSerialNumber;
    BYTE    bNumConfigurations;
} DEVICE_DESCRIPTOR, *PDEVICE_DESCRIPTOR;

/// SPI Command Word
typedef struct SPI_CMD
{
    WORD   Cmd;
    WORD   SubCmd;
    DWORD  Len;
    DWORD  blockSize;
    DWORD  timeoutMs;
    DWORD  res;
} SPI_CMD, *PSPI_CMD;


// Bitfield masks for delay_mode structure element
#define SPI_INTERBYTE_DELAY_MASK        0x01
#define SPI_CS_POSTASSERT_DELAY_MASK    0x02
#define SPI_CS_PREDEASSERT_DELAY_MASK   0x04
#define SPI_CS_TOGGLE_MASK              0x08

/// @}

/////////////////////////////////////////////////////////////////////////////
// Exported API Functions
/////////////////////////////////////////////////////////////////////////////

/// @name Exported API Functions
/// @{

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

USB_SPI_STATUS
CP213x_GetLibraryVersion( BYTE* major, BYTE* minor, BOOL* release );

//
//  CP2130 Device Management
//
/// @name CP2130 Device Management
/// @{


USB_SPI_STATUS
CP213x_GetNumDevices            ( DWORD* numDevices, DWORD VID, DWORD PID );


USB_SPI_STATUS
CP213x_Open                     ( DWORD deviceIndex, CP213x_DEVICE* phDevice, DWORD VID, DWORD PID );

USB_SPI_STATUS
CP213x_Close                    ( CP213x_DEVICE hDevice );

BOOL 
CP213x_IsOpened                 ( CP213x_DEVICE hDevice);

USB_SPI_STATUS 
CP213x_Reset                    ( CP213x_DEVICE hDevice );

USB_SPI_STATUS 
CP213x_GetDeviceVersion         ( CP213x_DEVICE hDevice, BYTE* majorVersion, BYTE* minorVersion );

USB_SPI_STATUS 
CP213x_GetDeviceDescriptor      ( CP213x_DEVICE hDevice,
                                  PDEVICE_DESCRIPTOR pDescriptor );

USB_SPI_STATUS 
CP213x_GetStringDescriptor      ( CP213x_DEVICE hDevice,
                                  BYTE index,
                                  BYTE stringDescriptor[STRING_DESCRIPTOR_SIZE] );

USB_SPI_STATUS 
CP213x_GetUsbConfig             ( CP213x_DEVICE hDevice, 
                                  WORD* vid,
                                  WORD* pid,
                                  BYTE* power,
                                  BYTE* powerMode,
                                  WORD* releaseVersion,
                                  BYTE* transferPriority );


USB_SPI_STATUS 
CP213x_SetUsbConfig             ( CP213x_DEVICE hDevice,
                                  WORD vid,
                                  WORD pid,
                                  BYTE power,
                                  BYTE powerMode,
                                  WORD releaseVersion,
                                  BYTE transferPriority,
                                  BYTE mask );

								  
USB_SPI_STATUS 
CP213x_GetManufacturingString   ( CP213x_DEVICE hDevice, LPSTR manufacturingString, BYTE* strlen );

USB_SPI_STATUS 
CP213x_SetManufacturingString   ( CP213x_DEVICE hDevice, LPCSTR manufacturingString, BYTE strlen );

USB_SPI_STATUS 
CP213x_GetProductString         ( CP213x_DEVICE hDevice, LPSTR productString, BYTE* strlen );

USB_SPI_STATUS 
CP213x_SetProductString         ( CP213x_DEVICE hDevice, LPCSTR productString, BYTE strlen );

USB_SPI_STATUS 
CP213x_GetSerialString          ( CP213x_DEVICE hDevice, LPSTR serialString, BYTE* strlen );

USB_SPI_STATUS 
CP213x_SetSerialString          ( CP213x_DEVICE hDevice, LPCSTR serialString, BYTE strlen );

USB_SPI_STATUS 
CP213x_GetPinConfig             ( CP213x_DEVICE hDevice, BYTE pinConfig[SIZE_PIN_CONFIG] );

USB_SPI_STATUS 
CP213x_SetPinConfig             ( CP213x_DEVICE hDevice, BYTE pinConfig[SIZE_PIN_CONFIG] );

USB_SPI_STATUS 
CP213x_GetLock                  ( CP213x_DEVICE hDevice, WORD* lockValue );

USB_SPI_STATUS 
CP213x_SetLock                  ( CP213x_DEVICE hDevice, WORD lockValue );

USB_SPI_STATUS 
CP213x_ReadProm                 ( CP213x_DEVICE hDevice, BYTE pReadBuf[] );

USB_SPI_STATUS 
CP213x_WriteProm                ( CP213x_DEVICE hDevice, BYTE pWriteBuf[] );
/// @}

//
//  CP2130 SPI Configuration and Transfer Operations
//
/// @name CP2130 SPI Configuration and Transfer Operations
/// @{

USB_SPI_STATUS 
CP213x_GetSpiControlBytes       ( CP213x_DEVICE hDevice, BYTE controlBytes[CP213x_NUM_GPIO] );

USB_SPI_STATUS 
CP213x_SetSpiControlByte        ( CP213x_DEVICE hDevice, BYTE channel, BYTE controlByte );

USB_SPI_STATUS 
CP213x_GetSpiDelay              ( CP213x_DEVICE hDevice,
                                    BYTE channel,
                                    BYTE* delayMode,
                                    WORD* interByteDelay,
                                    WORD* postAssertDelay,
                                    WORD* preDeassertDelay );

USB_SPI_STATUS 
CP213x_SetSpiDelay              ( CP213x_DEVICE hDevice,
                                  BYTE channel,
                                  BYTE delayMode,
                                  WORD interByteDelay,
                                  WORD postAssertDelay,
                                  WORD preDeassertDelay );
//not tested
USB_SPI_STATUS 
CP213x_GetChipSelect            ( CP213x_DEVICE hDevice,
                                  WORD* channelCsEnable,
                                  WORD* pinCsEnable );
//not tested
USB_SPI_STATUS 
CP213x_SetChipSelect            ( CP213x_DEVICE hDevice, BYTE channel, BYTE mode );
//not tested
USB_SPI_STATUS 
CP213x_TransferWrite            ( CP213x_DEVICE hDevice,
                                  BYTE pWriteBuf[],
                                  DWORD length,
                                  BOOL releaseBusAfterTransfer,
                                  DWORD timeoutMs,
                                  DWORD* pBytesActuallyWritten );
//not tested
USB_SPI_STATUS 
CP213x_TransferWriteRead        ( CP213x_DEVICE hDevice,
                                  BYTE pWriteBuf[],
                                  BYTE pReadBuf[],
                                  DWORD length,
                                  BOOL releaseBusAfterTransfer,
                                  DWORD timeoutMs,
                                  DWORD* pBytesActuallyTransferred );
//not tested
USB_SPI_STATUS 
CP213x_TransferReadAsync        ( CP213x_DEVICE hDevice,
                                  DWORD totalSize,
                                  DWORD blockSize,
                                  BOOL releaseBusAfterTransfer );

USB_SPI_STATUS 
CP213x_TransferReadSync         ( CP213x_DEVICE hDevice,
                                  BYTE pReadBuf[],
                                  DWORD length,
                                  BOOL releaseBusAfterTransfer,
                                  DWORD timeoutMs,
                                  DWORD* pBytesActuallyRead );

USB_SPI_STATUS 
CP213x_TransferReadRtrAsync     ( CP213x_DEVICE hDevice,
                                  DWORD totalSize,
                                  DWORD blockSize,
                                  BOOL releaseBusAfterTransfer );

USB_SPI_STATUS 
CP213x_TransferReadRtrSync      ( CP213x_DEVICE hDevice,
                                  BYTE pReadBuf[],
                                  DWORD totalSize,
                                  DWORD blockSize,
                                  BOOL releaseBusAfterTransfer,
                                  DWORD timeoutMs,
                                  DWORD* pBytesActuallyRead );

USB_SPI_STATUS 
CP213x_GetRtrState              ( CP213x_DEVICE hDevice, BYTE* isStopped );

USB_SPI_STATUS 
CP213x_SetRtrStop               ( CP213x_DEVICE hDevice, BYTE stopRtr );

USB_SPI_STATUS 
CP213x_ReadPoll                 ( CP213x_DEVICE hDevice,
                                  BYTE pReadBuf[],
                                  DWORD maxLength,
                                  DWORD* pBytesActuallyRead );

USB_SPI_STATUS 
CP213x_ReadAbort                ( CP213x_DEVICE hDevice );

USB_SPI_STATUS 
CP213x_AbortInputPipe           ( CP213x_DEVICE hDevice );

USB_SPI_STATUS 
CP213x_FlushInputPipe           ( CP213x_DEVICE hDevice );

USB_SPI_STATUS 
CP213x_AbortOutputPipe          ( CP213x_DEVICE hDevice );

USB_SPI_STATUS 
CP213x_FlushOutputPipe          ( CP213x_DEVICE hDevice );

USB_SPI_STATUS 
CP213x_GetFifoFullThreshold     ( CP213x_DEVICE hDevice, BYTE* pFifoFullThreshold );

USB_SPI_STATUS 
CP213x_SetFifoFullThreshold     ( CP213x_DEVICE hDevice, BYTE fifoFullThreshold );

/// @}

//
//  CP2130 GPIO and Auxiliary-Function Pins
//
/// @name CP2130 GPIO and Auxiliary-Function Pins
/// @{

USB_SPI_STATUS 
CP213x_GetGpioModeAndLevel      ( CP213x_DEVICE hDevice, BYTE channel, BYTE* mode, BYTE* level );

USB_SPI_STATUS 
CP213x_SetGpioModeAndLevel      ( CP213x_DEVICE hDevice, BYTE channel, BYTE mode, BYTE level );

USB_SPI_STATUS 
CP213x_GetGpioValues            ( CP213x_DEVICE hDevice, WORD* gpioValues );

USB_SPI_STATUS 
CP213x_SetGpioValues            ( CP213x_DEVICE hDevice, WORD mask, WORD gpioValues );

USB_SPI_STATUS 
CP213x_GetEventCounter          ( CP213x_DEVICE hDevice, BYTE* mode, WORD* eventCount );

USB_SPI_STATUS 
CP213x_SetEventCounter          ( CP213x_DEVICE hDevice, BYTE mode, WORD eventCount );

USB_SPI_STATUS 
CP213x_GetClockDivider          ( CP213x_DEVICE hDevice, BYTE* clockDivider );

USB_SPI_STATUS 
CP213x_SetClockDivider          ( CP213x_DEVICE hDevice, BYTE clockDivider );

/// @}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLAB_USB_SPI_H

/////////////////////////////////////////////////////////////////////////////
// End of file
/////////////////////////////////////////////////////////////////////////////
