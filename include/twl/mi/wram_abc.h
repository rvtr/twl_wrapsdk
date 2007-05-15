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

#ifdef	SDK_ARM9
#include	<nitro/hw/ARM9/mmap_global.h>
#else  //SDK_ARM7
#include	<nitro/hw/ARM7/mmap_global.h>
#endif


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
    MI_WRAM_B_ARM9  = 0 << REG_MI_WRAM_B0_MST_SHIFT,
    MI_WRAM_B_ARM7  = 1 << REG_MI_WRAM_B0_MST_SHIFT,
    MI_WRAM_B_DSP_I = 2 << REG_MI_WRAM_B0_MST_SHIFT
}
MIWramB;

typedef enum
{
    MI_WRAM_C_ARM9  = 0 << REG_MI_WRAM_C0_MST_SHIFT,
    MI_WRAM_C_ARM7  = 1 << REG_MI_WRAM_C0_MST_SHIFT,
    MI_WRAM_C_DSP_D = 2 << REG_MI_WRAM_C0_MST_SHIFT
}
MIWramC;

typedef enum
{
    MI_WRAM_A_OFS_0KB   = 0 << REG_MI_WRAM_A0_OFS_SHIFT,
    MI_WRAM_A_OFS_64KB  = 1 << REG_MI_WRAM_A0_OFS_SHIFT,
    MI_WRAM_A_OFS_128KB = 2 << REG_MI_WRAM_A0_OFS_SHIFT,
    MI_WRAM_A_OFS_192KB = 3 << REG_MI_WRAM_A0_OFS_SHIFT
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
    MI_WRAM_A_IMG_64KB  = 1 << REG_MI_WRAM_A_MAP_IMG_SHIFT,
    MI_WRAM_A_IMG_128KB = 2 << REG_MI_WRAM_A_MAP_IMG_SHIFT,
    MI_WRAM_A_IMG_256KB = 3 << REG_MI_WRAM_A_MAP_IMG_SHIFT,

    MI_WRAM_A_IMG_MIN   = MI_WRAM_A_IMG_64KB,
#ifdef BROM_PLATFORM_BB
    MI_WRAM_A_IMG_MAX   = MI_WRAM_A_IMG_128KB
#else // BROM_PLATFORM_TS
    MI_WRAM_A_IMG_MAX   = MI_WRAM_A_IMG_256KB
#endif // BROM_PLATFORM_TS
}
MIImageWramA;

typedef enum
{
    MI_WRAM_B_IMG_32KB  = 0 << REG_MI_WRAM_B_MAP_IMG_SHIFT,
    MI_WRAM_B_IMG_64KB  = 1 << REG_MI_WRAM_B_MAP_IMG_SHIFT,
    MI_WRAM_B_IMG_128KB = 2 << REG_MI_WRAM_B_MAP_IMG_SHIFT,
    MI_WRAM_B_IMG_256KB = 3 << REG_MI_WRAM_B_MAP_IMG_SHIFT,

    MI_WRAM_B_IMG_MIN   = MI_WRAM_B_IMG_32KB,
#ifdef BROM_PLATFORM_BB
    MI_WRAM_B_IMG_MAX   = MI_WRAM_B_IMG_128KB
#else // BROM_PLATFORM_TS
    MI_WRAM_B_IMG_MAX   = MI_WRAM_B_IMG_256KB
#endif // BROM_PLATFORM_TS
}
MIImageWramB;

typedef enum
{
    MI_WRAM_C_IMG_32KB  = 0 << REG_MI_WRAM_C_MAP_IMG_SHIFT,
    MI_WRAM_C_IMG_64KB  = 1 << REG_MI_WRAM_C_MAP_IMG_SHIFT,
    MI_WRAM_C_IMG_128KB = 2 << REG_MI_WRAM_C_MAP_IMG_SHIFT,
    MI_WRAM_C_IMG_256KB = 3 << REG_MI_WRAM_C_MAP_IMG_SHIFT,

    MI_WRAM_C_IMG_MIN   = MI_WRAM_C_IMG_32KB,
#ifdef BROM_PLATFORM_BB
    MI_WRAM_C_IMG_MAX   = MI_WRAM_C_IMG_128KB
#else // BROM_PLATFORM_TS
    MI_WRAM_C_IMG_MAX   = MI_WRAM_C_IMG_256KB
#endif // BROM_PLATFORM_TS
}
MIImageWramC;


#define MI_WRAM_A_BLOCK_SIZE  0x00010000  // 64KB
#define MI_WRAM_B_BLOCK_SIZE  0x00008000  // 32KB
#define MI_WRAM_C_BLOCK_SIZE  0x00008000  // 32KB

#ifdef BROM_PLATFORM_BB
#define MI_WRAM_A_BLOCK_NUM   2
#define MI_WRAM_B_BLOCK_NUM   4
#define MI_WRAM_C_BLOCK_NUM   4
#else // BROM_PLATFORM_TS
#define MI_WRAM_A_BLOCK_NUM   4
#define MI_WRAM_B_BLOCK_NUM   8
#define MI_WRAM_C_BLOCK_NUM   8
#endif // BROM_PLATFORM_TS


#define REG_WRAM_A_BNK_PACK( b_no, master, ofs, enable )  REG_WRAM_BNK_PACK( A, b_no, (master), (ofs), (enable) )
#define REG_WRAM_B_BNK_PACK( b_no, master, ofs, enable )  REG_WRAM_BNK_PACK( B, b_no, (master), (ofs), (enable) )
#define REG_WRAM_C_BNK_PACK( b_no, master, ofs, enable )  REG_WRAM_BNK_PACK( C, b_no, (master), (ofs), (enable) )
#define REG_WRAM_BNK_PACK( abc, b_no, master, ofs, enable ) \
( \
    (((enable) != FALSE) * REG_MI_WRAM_##abc##b_no##_E_MASK) \
  | (ofs) \
  | (master) \
)


#define MI_WRAM_MAP_NULL        HW_WRAM_AREA

#define REG_MI_WRAM_A_MAP_MAX   0x10000000
#define REG_MI_WRAM_B_MAP_MAX   REG_MI_WRAM_A_MAP_MAX
#define REG_MI_WRAM_C_MAP_MAX   REG_MI_WRAM_A_MAP_MAX

#define REG_WRAM_A_MAP_PACK( start, end, img_size )  REG_WRAM_MAP_PACK( A, (start), (end), (img_size) )
#define REG_WRAM_B_MAP_PACK( start, end, img_size )  REG_WRAM_MAP_PACK( B, (start), (end), (img_size) )
#define REG_WRAM_C_MAP_PACK( start, end, img_size )  REG_WRAM_MAP_PACK( C, (start), (end), (img_size) )
#define REG_WRAM_MAP_PACK( abc, start, end, img_size ) \
( \
    (((((start) - HW_WRAM_AREA) / MI_WRAM_##abc##_BLOCK_SIZE) << REG_MI_WRAM_##abc##_MAP_START_SHIFT) & REG_MI_WRAM_##abc##_MAP_START_MASK) \
  | (((((end)   - HW_WRAM_AREA) / MI_WRAM_##abc##_BLOCK_SIZE) << REG_MI_WRAM_##abc##_MAP_END_SHIFT)   & REG_MI_WRAM_##abc##_MAP_END_MASK) \
  | (img_size) \
)

#define REG_WRAM_A_MAP_OFS_PACK( start_ofs, end_ofs )  REG_WRAM_MAP_OFS_PACK( A, (start_ofs), (end_ofs) )
#define REG_WRAM_B_MAP_OFS_PACK( start_ofs, end_ofs )  REG_WRAM_MAP_OFS_PACK( B, (start_ofs), (end_ofs) )
#define REG_WRAM_C_MAP_OFS_PACK( start_ofs, end_ofs )  REG_WRAM_MAP_OFS_PACK( C, (start_ofs), (end_ofs) )
#define REG_WRAM_MAP_OFS_PACK( abc, start_ofs, end_ofs ) \
( \
    ((((start_ofs) / MI_WRAM_##abc##_BLOCK_SIZE) << REG_MI_WRAM_##abc##_MAP_START_SHIFT) & REG_MI_WRAM_##abc##_MAP_START_MASK) \
  | ((((end_ofs)   / MI_WRAM_##abc##_BLOCK_SIZE) << REG_MI_WRAM_##abc##_MAP_END_SHIFT)   & REG_MI_WRAM_##abc##_MAP_END_MASK) \
)


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_MI_WRAM_ABC_H_ */
#endif
