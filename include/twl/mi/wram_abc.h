/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     wram_abc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_MI_WRAM_ABC_H_
#define TWL_MI_WRAM_ABC_H_

#include <twl/ioreg.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------
//    enum definition
//

typedef enum
{
    MI_WRAM_A_ARM9 = 0 << REG_MI_WRAM_A0_MST_SHIFT,
    MI_WRAM_A_ARM7 = 1 << REG_MI_WRAM_A0_MST_SHIFT
}
MIWramA;

typedef enum
{
    MI_WRAM_B_ARM9 = 0 << REG_MI_WRAM_B0_MST_SHIFT,
    MI_WRAM_B_ARM7 = 1 << REG_MI_WRAM_B0_MST_SHIFT,
    MI_WRAM_B_DSP  = 2 << REG_MI_WRAM_B0_MST_SHIFT
}
MIWramB;

typedef enum
{
    MI_WRAM_C_ARM9 = 0 << REG_MI_WRAM_C0_MST_SHIFT,
    MI_WRAM_C_ARM7 = 1 << REG_MI_WRAM_C0_MST_SHIFT,
    MI_WRAM_C_DSP  = 2 << REG_MI_WRAM_C0_MST_SHIFT
}
MIWramC;

typedef enum
{
    MI_WRAM_A_OFS_0KB   = 0 << REG_MI_WRAM_A0_OFS_SHIFT,
    MI_WRAM_A_OFS_64KB  = 1 << REG_MI_WRAM_A0_OFS_SHIFT,
    MI_WRAM_A_OFS_128KB = 2 << REG_MI_WRAM_A0_OFS_SHIFT,
    MI_WRAM_A_OFS_192KB = 1 << REG_MI_WRAM_A0_OFS_SHIFT
}
MIOfsWramA;

typedef enum
{
    MI_WRAM_B_OFS_0KB   = 0 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_32KB  = 1 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_64KB  = 2 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_96KB  = 3 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_128KB = 4 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_160KB = 5 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_192KB = 6 << REG_MI_WRAM_B0_OFS_SHIFT,
    MI_WRAM_B_OFS_224KB = 7 << REG_MI_WRAM_B0_OFS_SHIFT
}
MIOfsWramB;

typedef enum
{
    MI_WRAM_C_OFS_0KB   = 0 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_32KB  = 1 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_64KB  = 2 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_96KB  = 3 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_128KB = 4 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_160KB = 5 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_192KB = 6 << REG_MI_WRAM_C0_OFS_SHIFT,
    MI_WRAM_C_OFS_224KB = 7 << REG_MI_WRAM_C0_OFS_SHIFT
}
MIOfsWramC;

typedef enum
{
    MI_WRAM_A_IMG_64KB  = 0 << REG_MI_WRAM_A_MAP_IMG_SHIFT,
    MI_WRAM_A_IMG_128KB = 1 << REG_MI_WRAM_A_MAP_IMG_SHIFT,
    MI_WRAM_A_IMG_256KB = 2 << REG_MI_WRAM_A_MAP_IMG_SHIFT
}
MIImageWramA;

typedef enum
{
    MI_WRAM_B_IMG_32KB  = 0 << REG_MI_WRAM_B_MAP_IMG_SHIFT,
    MI_WRAM_B_IMG_64KB  = 1 << REG_MI_WRAM_B_MAP_IMG_SHIFT,
    MI_WRAM_B_IMG_128KB = 2 << REG_MI_WRAM_B_MAP_IMG_SHIFT,
    MI_WRAM_B_IMG_256KB = 3 << REG_MI_WRAM_B_MAP_IMG_SHIFT
}
MIImageWramB;

typedef enum
{
    MI_WRAM_C_IMG_32KB  = 0 << REG_MI_WRAM_C_MAP_IMG_SHIFT,
    MI_WRAM_C_IMG_64KB  = 1 << REG_MI_WRAM_C_MAP_IMG_SHIFT,
    MI_WRAM_C_IMG_128KB = 2 << REG_MI_WRAM_C_MAP_IMG_SHIFT,
    MI_WRAM_C_IMG_256KB = 3 << REG_MI_WRAM_C_MAP_IMG_SHIFT
}
MIImageWramC;


#define MI_WRAM_A_BLOCK_SIZE  0x00010000  // 64KB
#define MI_WRAM_B_BLOCK_SIZE  0x00008000  // 32KB
#define MI_WRAM_C_BLOCK_SIZE  0x00008000  // 32KB


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_MI_WRAM_ABC_H_ */
#endif
