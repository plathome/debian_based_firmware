/*
 * Axiomtek SBC84620 API Library
 * User-Mode Driver	
 * Wrote by ryan.hsu@axiomtek.com.tw
 */

/* Library ==> /usr/lib/libaxio.so.1.0.0 */
/**********************eapi header file********************/
typedef int __int32;
typedef unsigned int uint32_t;

typedef unsigned int EApiStatus_t;
typedef unsigned int EApiId_t;

#ifndef EAPI_UINT32_C
#  define EAPI_UINT8_C(x)  ((uint8_t)(x))
#  define EAPI_UINT16_C(x) ((uint16_t)(x))
#  define EAPI_UINT32_C(x) ((uint32_t)(x))
#endif

#define EAPI_STATUS_INVALID_PARAMETER EAPI_UINT32_C(0xFFFFFEFF)

/* Description
 *   The Block Alignment is incorrect.
 * Actions
 *   Use pInputs and pOutputs to correctly select input and outputs. 
 */
#define EAPI_STATUS_INVALID_BLOCK_ALIGNMENT EAPI_UINT32_C(0xFFFFFEFE)

/* Description
 *   This means that the Block length is too long.
 * Actions
 *   Use Alignment Capabilities information to correctly align write access.
 */
#define EAPI_STATUS_INVALID_BLOCK_LENGTH    EAPI_UINT32_C(0xFFFFFEFD)

/* Description
 *   The current Direction Argument attempts to set GPIOs to a unsupported 
 *   directions. I.E. Setting GPI to Output.
 * Actions
 *   Use pInputs and pOutputs to correctly select input and outputs. 
 */
#define EAPI_STATUS_INVALID_DIRECTION       EAPI_UINT32_C(0xFFFFFEFC)

/* Description
 *   The Bitmask Selects bits/GPIOs which are not supported for the current ID.
 * Actions
 *   Use pInputs and pOutputs to probe supported bits..
 */
#define EAPI_STATUS_INVALID_BITMASK         EAPI_UINT32_C(0xFFFFFEFB)

/* Description
 *   Watchdog timer already started.
 * Actions
 *   Call EApiWDogStop, before retrying.
 */
#define EAPI_STATUS_RUNNING                 EAPI_UINT32_C(0xFFFFFEFA)

/* Description
 *   This function or ID is not supported at the actual hardware environment.
 * Actions
 *   none.
 */
#define EAPI_STATUS_UNSUPPORTED       EAPI_UINT32_C(0xFFFFFCFF)

/* Description
 *   I2C Device Error
 *   No Acknowledge For Device Address, 7Bit Address Only
 *   10Bit Address may cause Write error if 2 10Bit addressed devices 
 *   present on the bus.
 * Actions
 *   none.
 */
#define EAPI_STATUS_NOT_FOUND         EAPI_UINT32_C(0xFFFFFBFF)

/* Description
 *   I2C Time-out
 *   Device Clock stretching time-out, Clock pulled low by device 
 *   for too long
 * Actions
 *   none.
 */
#define EAPI_STATUS_TIMEOUT           EAPI_UINT32_C(0xFFFFFBFE)

/* Description
 *   EApi  I2C functions specific. The addressed I2C bus is busy or there 
 *   is a bus collision.
 *   The I2C bus is in use. Either CLK or DAT are low.
 *   Arbitration loss or bus Collision, data remains low when writing a 1
 * Actions
 *   Retry.
 */
#define EAPI_STATUS_BUSY_COLLISION    EAPI_UINT32_C(0xFFFFFBFD)

/* Description
 *   I2C Read Error
 *    Not Possible to detect.
 *   Storage Read Error
 *    ....
 * Actions
 *   Retry.
 */
#define EAPI_STATUS_READ_ERROR        EAPI_UINT32_C(0xFFFFFAFF)
/*#define EAPI_STATUS_WRITE_ERROR        EAPI_UINT32_C(0xFFFFFAEF)*/


/* Description
 *   I2C Write Error
 *     No Acknowledge received after writing any Byte after the First Address 
 *     Byte.
 *     Can be caused by 
 *     unsupported Device Command/Index
 *     Ext Command/Index used on Standard Command/Index Device
 *     10Bit Address Device Not Present
 *   Storage Write Error
 *    ...
 * Actions
 *   Retry.
 */
#define EAPI_STATUS_WRITE_ERROR       EAPI_UINT32_C(0xFFFFFAFE)


/* Description
 *   The amount of available data exceeds the buffer size.
 *   Storage buffer overflow was prevented. Read count was larger then 
 *   the defined buffer length.
 *   Read Count > Buffer Length
 * Actions
 *   Either increase the buffer size or reduce the block length.
 */
#define EAPI_STATUS_MORE_DATA         EAPI_UINT32_C(0xFFFFF9FF)

/* Description
 *   Generic error message. No further error details are available.
 * Actions
 *   none.
 */
#define EAPI_STATUS_ERROR             EAPI_UINT32_C(0xFFFFF0FF)

/* Description
 *   The operation was successful.
 * Actions
 *   none.
 */
#define EAPI_STATUS_SUCCESS           EAPI_UINT32_C(0)



EApiStatus_t EApiBoardGetValue( EApiId_t  Id,uint32_t *pValue);

/*****************************************************/



/****************************************************/
/* Hardware Monitor                         */

EApiStatus_t AXBoardSetIndexValue(
                EApiId_t  Id,           /* Value Id */
                uint32_t Index,
                uint32_t pValue    /* Value */
    ); 
EApiStatus_t AXBoardSetValue(
		EApiId_t  Id,		/* Value Id */
		
		uint32_t pValue    /* Value */
    );
EApiStatus_t EApiBoardGetValue(
    EApiId_t  Id      , /* Value Id */
    uint32_t *pValue    /* Return Value */
);

//default 
#define EAPI_ID_HWMON_CPU_TEMP            EAPI_UINT32_C(0x20000) /* 0.1 */ 
#define EAPI_ID_HWMON_CHIPSET_TEMP        EAPI_UINT32_C(0x20001) /* 0.1 */ 
#define EAPI_ID_HWMON_SYSTEM1_TEMP         EAPI_UINT32_C(0x20002) /* 0.1 */ 
#define EAPI_ID_HWMON_SYSTEM2_TEMP         EAPI_UINT32_C(0x20003) /* 0.1 */ 

#define EAPI_ID_HWMON_VOLTAGE_VCORE        EAPI_UINT32_C(0x21001)
#define EAPI_ID_HWMON_VOLTAGE_3V3          EAPI_UINT32_C(0x21002)
#define EAPI_ID_HWMON_VOLTAGE_5V0          EAPI_UINT32_C(0x21003)
#define EAPI_ID_HWMON_VOLTAGE_12V0         EAPI_UINT32_C(0x21004)
#define EAPI_ID_HWMON_VOLTAGE_3V3SB        EAPI_UINT32_C(0x21005) 
#define EAPI_ID_HWMON_VOLTAGE_5V0SB        EAPI_UINT32_C(0x21006) 
#define EAPI_ID_HWMON_VOLTAGE_VBAT	   EAPI_UINT32_C(0x21007)

#define EAPI_ID_HWMON_FAN_CPU		  EAPI_UINT32_C(0x22001)
#define EAPI_ID_HWMON_FAN_SYS1             EAPI_UINT32_C(0x22002)
#define EAPI_ID_HWMON_FAN_SYS2             EAPI_UINT32_C(0x22003)


//AX
#define EAPI_AX_HWMON_MAIN_DIO_DIR				EAPI_UINT32_C(0x31000)
#define EAPI_AX_HWMON_MAIN_DIO_NUM				EAPI_UINT32_C(0x31001)
#define EAPI_AX_HWMON_SLOT1_DI_NUM                              EAPI_UINT32_C(0x31002)
#define EAPI_AX_HWMON_SLOT1_DO_NUM                              EAPI_UINT32_C(0x31003)
#define EAPI_AX_HWMON_SLOT2_DI_NUM                              EAPI_UINT32_C(0x31004)
#define EAPI_AX_HWMON_SLOT2_DO_NUM                              EAPI_UINT32_C(0x31005)

#define EAPI_AX_HWMON_SLOT3_DI_NUM                              EAPI_UINT32_C(0x31006)
#define EAPI_AX_HWMON_SLOT3_DO_NUM                              EAPI_UINT32_C(0x31007)
#define EAPI_AX_HWMON_SLOT4_DI_NUM                              EAPI_UINT32_C(0x31008)
#define EAPI_AX_HWMON_SLOT4_DO_NUM                              EAPI_UINT32_C(0x31009)
#define EAPI_AX_HWMON_SLOT5_DI_NUM                              EAPI_UINT32_C(0x3100a)
#define EAPI_AX_HWMON_SLOT5_DO_NUM                              EAPI_UINT32_C(0x3100b)
#define EAPI_AX_HWMON_SLOT6_DI_NUM                              EAPI_UINT32_C(0x3100c)
#define EAPI_AX_HWMON_SLOT6_DO_NUM                              EAPI_UINT32_C(0x3100d)
#define EAPI_AX_HWMON_SLOT7_DI_NUM                              EAPI_UINT32_C(0x3100e)
#define EAPI_AX_HWMON_SLOT7_DO_NUM                              EAPI_UINT32_C(0x3100f)
#define EAPI_AX_HWMON_SLOT8_DI_NUM                              EAPI_UINT32_C(0x31010)
#define EAPI_AX_HWMON_SLOT8_DO_NUM                              EAPI_UINT32_C(0x31011)

#define EAPI_AX_HWMON_DI0				  EAPI_UINT32_C(0x31100)
#define EAPI_AX_HWMON_DI1				  EAPI_UINT32_C(0x31101)
#define EAPI_AX_HWMON_DI2				  EAPI_UINT32_C(0x31102)
#define EAPI_AX_HWMON_DI3				  EAPI_UINT32_C(0x31103)

#define EAPI_AX_HWMON_DI4				  EAPI_UINT32_C(0x31104)
#define EAPI_AX_HWMON_DI5				  EAPI_UINT32_C(0x31105)
#define EAPI_AX_HWMON_DI6				  EAPI_UINT32_C(0x31106)
#define EAPI_AX_HWMON_DI7				  EAPI_UINT32_C(0x31107)
#define EAPI_AX_HWMON_DI8				  EAPI_UINT32_C(0x31108)
#define EAPI_AX_HWMON_DI9				  EAPI_UINT32_C(0x31109)
#define EAPI_AX_HWMON_DI10				  EAPI_UINT32_C(0x3110a)
#define EAPI_AX_HWMON_DI11				  EAPI_UINT32_C(0x3110b)
#define EAPI_AX_HWMON_DI12				  EAPI_UINT32_C(0x3110c)
#define EAPI_AX_HWMON_DI13				  EAPI_UINT32_C(0x3110d)

#define EAPI_AX_HWMON_DI14				  EAPI_UINT32_C(0x3110e)
#define EAPI_AX_HWMON_DI15				  EAPI_UINT32_C(0x3110f)
#define EAPI_AX_HWMON_DI16				  EAPI_UINT32_C(0x31110)
#define EAPI_AX_HWMON_DI17				  EAPI_UINT32_C(0x31111)
#define EAPI_AX_HWMON_DI18				  EAPI_UINT32_C(0x31112)
#define EAPI_AX_HWMON_DI19				  EAPI_UINT32_C(0x31113)
#define EAPI_AX_HWMON_DI20				  EAPI_UINT32_C(0x31114)
#define EAPI_AX_HWMON_DI21				  EAPI_UINT32_C(0x31115)
#define EAPI_AX_HWMON_DI22				  EAPI_UINT32_C(0x31116)
#define EAPI_AX_HWMON_DI23				  EAPI_UINT32_C(0x31117)

#define EAPI_AX_HWMON_DI24				  EAPI_UINT32_C(0x31118)
#define EAPI_AX_HWMON_DI25				  EAPI_UINT32_C(0x31119)
#define EAPI_AX_HWMON_DI26				  EAPI_UINT32_C(0x3111a)
#define EAPI_AX_HWMON_DI27				  EAPI_UINT32_C(0x3111b)
#define EAPI_AX_HWMON_DI28				  EAPI_UINT32_C(0x3111c)
#define EAPI_AX_HWMON_DI29				  EAPI_UINT32_C(0x3111d)
#define EAPI_AX_HWMON_DI30				  EAPI_UINT32_C(0x3111e)
#define EAPI_AX_HWMON_DI31				  EAPI_UINT32_C(0x3111f)


#define EAPI_AX_HWMON_DO0				  EAPI_UINT32_C(0x31200)
#define EAPI_AX_HWMON_DO1				  EAPI_UINT32_C(0x31201)
#define EAPI_AX_HWMON_DO2				  EAPI_UINT32_C(0x31202)
#define EAPI_AX_HWMON_DO3				  EAPI_UINT32_C(0x31203)
#define EAPI_AX_HWMON_DO4				  EAPI_UINT32_C(0x31204)
#define EAPI_AX_HWMON_DO5				  EAPI_UINT32_C(0x31205)
#define EAPI_AX_HWMON_DO6				  EAPI_UINT32_C(0x31206)
#define EAPI_AX_HWMON_DO7				  EAPI_UINT32_C(0x31207)
#define EAPI_AX_HWMON_DO8				  EAPI_UINT32_C(0x31208)
#define EAPI_AX_HWMON_DO9				  EAPI_UINT32_C(0x31209)
#define EAPI_AX_HWMON_DO10				  EAPI_UINT32_C(0x3120a)
#define EAPI_AX_HWMON_DO11				  EAPI_UINT32_C(0x3120b)
#define EAPI_AX_HWMON_DO12				  EAPI_UINT32_C(0x3120c)
#define EAPI_AX_HWMON_DO13				  EAPI_UINT32_C(0x3120d)
#define EAPI_AX_HWMON_DO14				  EAPI_UINT32_C(0x3120e)
#define EAPI_AX_HWMON_DO15				  EAPI_UINT32_C(0x3120f)
#define EAPI_AX_HWMON_DO16				  EAPI_UINT32_C(0x31210)
#define EAPI_AX_HWMON_DO17				  EAPI_UINT32_C(0x31211)
#define EAPI_AX_HWMON_DO18				  EAPI_UINT32_C(0x31212)
#define EAPI_AX_HWMON_DO19				  EAPI_UINT32_C(0x31213)
#define EAPI_AX_HWMON_DO20				  EAPI_UINT32_C(0x31214)
#define EAPI_AX_HWMON_DO21				  EAPI_UINT32_C(0x31215)
#define EAPI_AX_HWMON_DO22				  EAPI_UINT32_C(0x31216)
#define EAPI_AX_HWMON_DO23				  EAPI_UINT32_C(0x31217)
#define EAPI_AX_HWMON_DO24				  EAPI_UINT32_C(0x31218)
#define EAPI_AX_HWMON_DO25				  EAPI_UINT32_C(0x31219)
#define EAPI_AX_HWMON_DO26				  EAPI_UINT32_C(0x3121a)
#define EAPI_AX_HWMON_DO27				  EAPI_UINT32_C(0x3121b)
#define EAPI_AX_HWMON_DO28				  EAPI_UINT32_C(0x3121c)
#define EAPI_AX_HWMON_DO29				  EAPI_UINT32_C(0x3121d)
#define EAPI_AX_HWMON_DO30				  EAPI_UINT32_C(0x3121e)
#define EAPI_AX_HWMON_DO31				  EAPI_UINT32_C(0x3121f)



#define EAPI_AX_HWMON_COM_INIT			  EAPI_UINT32_C(0x32199)
#define EAPI_AX_HWMON_COM1                EAPI_UINT32_C(0x32100)
#define EAPI_AX_HWMON_COM2                EAPI_UINT32_C(0x32101)
#define EAPI_AX_HWMON_COM3                EAPI_UINT32_C(0x32102)
#define EAPI_AX_HWMON_COM4                EAPI_UINT32_C(0x32103)
#define EAPI_AX_HWMON_COM5                EAPI_UINT32_C(0x32104)
#define EAPI_AX_HWMON_COM6                EAPI_UINT32_C(0x32105)

#define EAPI_AX_HWMON_COM1_TERM           EAPI_UINT32_C(0x32106)
#define EAPI_AX_HWMON_COM2_TERM           EAPI_UINT32_C(0x32107)
#define EAPI_AX_HWMON_COM3_TERM           EAPI_UINT32_C(0x32108)
#define EAPI_AX_HWMON_COM4_TERM           EAPI_UINT32_C(0x32109)
#define EAPI_AX_HWMON_COM5_TERM           EAPI_UINT32_C(0x3210A)
#define EAPI_AX_HWMON_COM6_TERM           EAPI_UINT32_C(0x3210B)

#define EAPI_AX_HWMON_WDT_ENABLE           EAPI_UINT32_C(0x33000)
#define EAPI_AX_HWMON_WDT_RELOAD           EAPI_UINT32_C(0x33001)
#define EAPI_AX_HWMON_WDT_STOP           EAPI_UINT32_C(0x33002)

#define EAPI_AX_HWMON_MODEL_NAME		EAPI_UINT32_C(0x40000)
#define EAPI_AX_HWMON_BLCTL				EAPI_UINT32_C(0x50000)

#define RS232 1
#define RS485 2
#define RS422 3

#define ENABLE 1
#define DISABLE 0

#define HIGH 1
#define LOW 0
#define  termination
#define TERMINATION_EN 1
#define TERMINATION_DIS 0




/*
 * Axiomtek ICO300 API Library
 * User-Mode Driver	
 * Wrote by joechen@axiomtek.com.tw
 */

/* Library ==> /usr/lib/libico300.so.1.0.0 */

/****************************************************/
/* Hardware Monitor Function                        */
/* timeout: range 0~255	                            */
/* Return -1 ,WDT enable failure                    */
/* otherwise, WDT enable success                    */
int ICO300_WDT_enable(unsigned char scale, unsigned char timeout);
/****************************************************/
/* disable the WDT timer                            */
int ICO300_WDT_disable(void);
/****************************************************/
/* reload the WDT timer                             */
int ICO300_WDT_reload(void);
/****************************************************/
/* read the WDT current setting value               */
/* return -1, the WDT not setting/enable yet        */
/* otherwise, the value store in *sec               */
int ICO300_read_WDT_config(unsigned int *time);
/****************************************************/

EApiStatus_t AXBoardGetIndexValue(
                EApiId_t  Id,           /* Value Id */
                uint32_t Index,
                uint32_t *pValue    /* Value */
    ); 
