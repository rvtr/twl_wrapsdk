/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     system.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_SYSTEM_H_
#define TWL_OS_SYSTEM_H_

#include <twl/misc.h>
#include <twl/types.h>

#include <nitro/os/common/system.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

typedef enum
{
    OS_CHIPTYPE_PRODUCT = 0,
    OS_CHIPTYPE_DEBUGGER_BOTH = 1,
    OS_CHIPTYPE_DEBUGGER_ARM9 = 2,
    OS_CHIPTYPE_EVALUATE = 3
}
OSChipType;

//---- entry point type
typedef void (*OSEntryPoint) (void);

typedef u32 OSCpuCycle;

#define OS_CPU_CLOCK                   HW_CPU_CLOCK

//---- sec to cpu cycle
// 150nsec - 30sec
#define  OS_SEC_TO_CPUCYC(  sec  ) ((OSCpuCycle)( ( OS_CPU_CLOCK * (u32)(sec)) ))
#define  OS_MSEC_TO_CPUCYC( msec ) ((OSCpuCycle)( ((OS_CPU_CLOCK/1000) * (u32)(msec)) ))
#define  OS_USEC_TO_CPUCYC( usec ) ((OSCpuCycle)( ((OS_CPU_CLOCK/1000) * (u32)(usec)) / 1000 ))
#define  OS_NSEC_TO_CPUCYC( nsec ) ((OSCpuCycle)( ((OS_CPU_CLOCK/1000) * (u32)(nsec)) / (1000 * 1000) ))

//---- cpu cycle to sec
// 150nsec - 30sec
#define  OS_CPUCYC_TO_SEC(  cyc ) ( ((u32)(cyc) ) / OS_CPU_CLOCK )
#define  OS_CPUCYC_TO_MSEC( cyc ) ( ((u32)(cyc) ) / (OS_CPU_CLOCK/1000) )
#define  OS_CPUCYC_TO_USEC( cyc ) ( ((u32)(cyc) * 1000) / (OS_CPU_CLOCK/1000) )
#define  OS_CPUCYC_TO_NSEC( cyc ) ( ((u32)(cyc) * 1000 * 1000) / (OS_CPU_CLOCK/1000) )


#ifdef SDK_ARM9

typedef enum
{
    OS_SPEED_ARM9_X1 = 0,
    OS_SPEED_ARM9_X2 = REG_CFG_CLK_ARM2X_MASK
}
OSSpeedOfARM9;

/*---------------------------------------------------------------------------*
  Name:         OS_ChangeSpeedOfARM9

  Description:  change speed of arm9

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_ChangeSpeedOfARM9( OSSpeedOfARM9 clock, void* itcm );

/*---------------------------------------------------------------------------*
  Name:         OS_IsARM9x2

  Description:  speed of arm9

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
inline BOOL OS_IsARM9x2( void )
{
    return (reg_CFG_CLK & REG_CFG_CLK_ARM2X_MASK) >> REG_CFG_CLK_ARM2X_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsDSPOn

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
inline BOOL OS_IsDSPOn( void )
{
    return (reg_CFG_CLK & REG_CFG_CLK_DSP_MASK) >> REG_CFG_CLK_DSP_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsDSPReset

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
inline BOOL OS_IsDSPReset( void )
{
    return TRUE ^ ((reg_CFG_DSP_RST & REG_CFG_DSP_RST_OFF_MASK) >> REG_CFG_DSP_RST_OFF_SHIFT);
}

#else // SDK_ARM7

/*---------------------------------------------------------------------------*
  Name:         OS_GetChipType

  Description:  get chip type

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
inline OSChipType OS_GetChipType( void )
{
    return (OSChipType)((reg_CFG_BONDING & REG_CFG_BONDING_CHIP_TYPE_MASK) >> REG_CFG_BONDING_CHIP_TYPE_SHIFT);
}

#endif // SDK_ARM7


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEM_H_ */
#endif
