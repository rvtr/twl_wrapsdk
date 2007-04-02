/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI
  File:     spi.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SPI_SPI_H_
#define TWL_SPI_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/ioreg.h>
#include <nitro/hw/ARM7/ioreg_SPI.h>

//----------------------------------------------------------------
//        enums
//
//---- SPI baud rate
typedef enum
{
    SPI_BAUDRATE_4MHZ = 0,
    SPI_BAUDRATE_2MHZ = 1,
    SPI_BAUDRATE_1MHZ = 2,
    SPI_BAUDRATE_512KHZ = 3,
    SPI_BAUDRATE_8MHZ = 4
}
SPIBaudRate;

//---- SPI baud rate
typedef enum
{
    SPI_TRANSMODE_1BYTE = 0,
    SPI_TRANSMODE_CONTINUOUS = 1
}
SPITransMode;

//---- Clock mode
#ifdef SDK_TS
typedef enum
{
    SPI_CLKMODE_8CLK = 0,
    SPI_CLKMODE_16CLK = 1
}
SPIClockMode;
#endif

//---- communication partner
#ifdef SDK_TS
typedef enum
{
    SPI_COMMPARTNER_PMIC = 0,
    SPI_COMMPARTNER_EEPROM = 1,
    SPI_COMMPARTNER_TP = 2
}
SPICommPartner;

#else  /* !SDK_TS */
typedef enum
{
    SPI_COMMPARTNER_PMIC = 0,
    SPI_COMMPARTNER_EEPROM = 1
}
SPICommPartner;

#define SPI_COMMPARTNER_TP      (SPI_COMMPARTNER_PMIC)
#endif


//----------------------------------------------------------------
//        assertion definition
//
#define SPI_COMMPARTNER_ASSERT( x )   SDK_ASSERT( (u32)x <= SPI_COMMPARTNER_EEPROM )
#define SPI_TRANSMODE_ASSERT( x )   SDK_ASSERT( (u32)x <= SPI_TRANSMODE_CONTINUOUS )
#define SPI_BAUDRATE_ASSERT( x )   SDK_ASSERT( (u32)x <= SPI_BAUDRATE_512KHZ )


//----------------------------------------------------------------
//---- enable/disable/restore SPIEnable
static inline BOOL SPI_Enable(void)
{
    BOOL    pre = (reg_SPI_SPICNT & ~REG_SPI_SPICNT_E_MASK) >> REG_SPI_SPICNT_E_SHIFT;
    reg_SPI_SPICNT =
        (u16)((reg_SPI_SPICNT & ~REG_SPI_SPICNT_E_MASK) | (1UL << REG_SPI_SPICNT_E_SHIFT));
    return pre;
}

static inline BOOL SPI_Disable(void)
{
    BOOL    pre = (reg_SPI_SPICNT & ~REG_SPI_SPICNT_E_MASK) >> REG_SPI_SPICNT_E_SHIFT;
    reg_SPI_SPICNT = (u16)((reg_SPI_SPICNT & ~REG_SPI_SPICNT_E_MASK));
    return pre;
}

static inline BOOL SPI_Restore(BOOL flag)
{
    BOOL    pre = (reg_SPI_SPICNT & ~REG_SPI_SPICNT_E_MASK) >> REG_SPI_SPICNT_E_SHIFT;
    reg_SPI_SPICNT =
        (u16)((reg_SPI_SPICNT & ~REG_SPI_SPICNT_E_MASK) |
              (flag ? 1UL << REG_SPI_SPICNT_E_SHIFT : 0));
    return pre;
}

//----------------------------------------------------------------
//---- Set/Get Interrupt 
static inline void SPI_SetInterrupt(BOOL e)
{
    reg_SPI_SPICNT =
        (u16)((reg_SPI_SPICNT & REG_SPI_SPICNT_I_MASK) | (e ? 1UL << REG_SPI_SPICNT_I_SHIFT : 0));
}

static inline BOOL SPI_GetInterrupt(void)
{
    return (BOOL)((reg_SPI_SPICNT & REG_SPI_SPICNT_I_MASK) >> REG_SPI_SPICNT_I_SHIFT);
}

//----------------------------------------------------------------
//---- Set/Get comminucation partner
static inline void SPI_SetCommPartner(SPICommPartner p)
{
    SPI_COMMPARTNER_ASSERT(p);
    reg_SPI_SPICNT =
        (u16)((reg_SPI_SPICNT & REG_SPI_SPICNT_SEL_MASK) | (p << REG_SPI_SPICNT_SEL_SHIFT));
}

static inline SPICommPartner SPI_GetCommPartner(void)
{
    return (SPICommPartner)((reg_SPI_SPICNT & REG_SPI_SPICNT_SEL_MASK) >> REG_SPI_SPICNT_SEL_SHIFT);
}

//----------------------------------------------------------------
//---- set/Get Transfer mode
static inline void SPI_SetTransMode(SPITransMode mode)
{
    SPI_TRANSMODE_ASSERT(mode);
    reg_SPI_SPICNT =
        (u16)((reg_SPI_SPICNT & REG_SPI_SPICNT_MODE_MASK) | (mode << REG_SPI_SPICNT_MODE_SHIFT));
}

static inline SPITransMode SPI_GetTransMode(void)
{
    return (SPITransMode)((reg_SPI_SPICNT & REG_SPI_SPICNT_MODE_MASK) >> REG_SPI_SPICNT_MODE_SHIFT);
}

//----------------------------------------------------------------
//---- check SPI is busy
static inline BOOL SPI_IsBusy(void)
{
    return (BOOL)((reg_SPI_SPICNT & REG_SPI_SPICNT_BUSY_MASK) >> REG_SPI_SPICNT_BUSY_SHIFT);
}

//----------------------------------------------------------------
//---- set/get Baudrate
static inline void SPI_SetBaudRate(SPIBaudRate baud)
{
    SPI_BAUDRATE_ASSERT(baud);
    reg_SPI_SPICNT =
        (u16)((reg_SPI_SPICNT & REG_SPI_SPICNT_BAUDRATE_MASK) |
              (baud << REG_SPI_SPICNT_BAUDRATE_SHIFT));
}

static inline SPIBaudRate SPI_GetBaudRate(void)
{
    return (SPIBaudRate)((reg_SPI_SPICNT & REG_SPI_SPICNT_BAUDRATE_MASK) >>
                         REG_SPI_SPICNT_BAUDRATE_SHIFT);
}

//----------------------------------------------------------------
//---- set/get data to/from SPI
static inline void SPI_SendData(u8 data)
{
    reg_SPI_SPID = (u16)data;
}

static inline u8 SPI_GetData(void)
{
    return (u8)(reg_SPI_SPICNT & 0xff);
}

//----------------------------------------------------------------
//        subroutine definition
//
void    SPI_Init(u32 prio);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_SPI_SPI_H_ */
#endif
