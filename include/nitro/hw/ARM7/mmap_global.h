/*---------------------------------------------------------------------------*
  Project:  NitroSDK - include/sp - HW
  File:     mmap_global.h

  Copyright 2003-2005 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: mmap_global.h,v $
  Revision 1.13  2005/02/28 05:26:27  yosizaki
  do-indent.

  Revision 1.12  2004/06/11 06:17:48  yada
  close SDK_BB support

  Revision 1.11  2004/04/06 06:38:30  yasu
  small fix for WRAM address

  Revision 1.10  2004/04/05 10:33:49  takano_makoto
  Small modyfy at indent.

  Revision 1.9  2004/04/05 10:32:34  takano_makoto
  Add HW_EXT_WRAM

  Revision 1.8  2004/03/25 01:29:45  yada
  only add comment

  Revision 1.7  2004/03/23 07:41:29  yada
  TEGでないときの、ARM7専用WRAMを考慮

  Revision 1.6  2004/02/17 08:22:07  yada
  tab整形

  Revision 1.5  2004/02/13 10:40:18  yada
  (none)

  Revision 1.4  2004/02/12 13:32:25  yasu
  change include guards

  Revision 1.3  2004/02/05 07:09:03  yasu
  change SDK prefix iris -> nitro

  Revision 1.2  2003/12/22 14:08:52  yasu
  include ガードの修正

  Revision 1.1  2003/12/16 10:56:38  yasu
  spcode から移動

  Revision 1.2  2003/12/16 06:29:41  ida
  DTCM の記述を削除
  内部ワークRAMの記述を追加

  Revision 1.1  2003/12/11 05:38:05  yasu
  ARM9 版をコピー

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_ARM7_HW_MMAP_GLOBAL_H_
#define NITRO_ARM7_HW_MMAP_GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// GLOBAL MEMORY MAP
//---------------------------------------------------------------------------

//------------------------------------------------
//  Physical Address
//
//----------------------------- MAIN
#define HW_MAIN_MEM             0x02000000
#define HW_MAIN_MEM_SIZE        0x00400000
#define HW_MAIN_MEM_EX_SIZE     0x01000000

#define HW_MAIN_MEM_END         (HW_MAIN_MEM + HW_MAIN_MEM_SIZE)
#define HW_MAIN_MEM_EX_END      (HW_MAIN_MEM + HW_MAIN_MEM_EX_SIZE)

//----------------------------- MAIN_HI
#define HW_MAIN_MEM_HI          0x0c000000
#define HW_MAIN_MEM_HI_SIZE     0x01000000
#define HW_MAIN_MEM_HI_EX_SIZE  0x02000000

#define HW_MAIN_MEM_HI_END      (HW_MAIN_MEM_HI + HW_MAIN_MEM_HI_SIZE)
#define HW_MAIN_MEM_HI_EX_END   (HW_MAIN_MEM_HI + HW_MAIN_MEM_HI_EX_SIZE)

//----------------------------- WRAM_AREA
#define HW_WRAM_AREA            0x03000000
#define HW_WRAM_AREA_END        0x04000000
#define HW_WRAM_AREA_SIZE       (HW_WRAM_AREA_END-HW_WRAM_AREA)

//----------------------------- WRAMs
#define HW_WRAM                 0x037f8000
#define HW_WRAM_END             0x03800000
#define HW_WRAM_SIZE            (HW_WRAM_END-HW_WRAM)

#define HW_WRAM_0               0x037f8000
#define HW_WRAM_0_END           0x037fc000
#define HW_WRAM_0_SIZE          (HW_WRAM_0_END-HW_WRAM_0)
#define HW_WRAM_1               0x037fc000
#define HW_WRAM_1_END           0x03800000
#define HW_WRAM_1_SIZE          (HW_WRAM_1_END-HW_WRAM_1)

#define HW_WRAM_EX              HW_PRV_WRAM_END
#ifdef TWL_PLATFORM_BB
#define HW_WRAM_A_SIZE_MAX      0x00020000
#else // TWL_PLATFORM_BB
#define HW_WRAM_A_SIZE_MAX      0x00040000
#endif // TWL_PLATFORM_BB
#define HW_WRAM_B_SIZE_MAX      HW_WRAM_A_SIZE_MAX
#define HW_WRAM_C_SIZE_MAX      HW_WRAM_B_SIZE_MAX

//----------------------------- PRV-WRAMs
#define HW_PRV_WRAM             0x03800000
// TEG   : ARM7 WRAM == 32KB
// TS    : ARM7 WRAM == 64KB
#if  defined(SDK_TEG)
#define HW_PRV_WRAM_END         0x03808000
#else
#define HW_PRV_WRAM_END         0x03810000
#endif
#define HW_PRV_WRAM_SIZE        (HW_PRV_WRAM_END-HW_PRV_WRAM)

//----------------------------- IOs
#define HW_IOREG                0x04000000
#define HW_IOREG_END            0x05000000
#define HW_REG_BASE             HW_IOREG        // alias

//----------------------------- VRAMs
#define HW_EXT_WRAM             0x06000000
#define HW_EXT_WRAM_END         0x06040000
#define HW_EXT_WRAM_SIZE        (HW_EXT_WRAM_END-HW_EXT_WRAM)

//----------------------------- Cartridge Bus
#define HW_CTRDG_ROM            0x08000000
#define HW_CTRDG_ROM_END        0x0a000000
#define HW_CTRDG_RAM            0x0a000000
#define HW_CTRDG_RAM_END        0x0a010000

//----------------------------- System ROM
#define HW_BIOS                 0x00000000
#define HW_BIOS_END             0x00004000

#define HW_RESET_VECTOR         0x00000000

#ifdef __cplusplus
} /* extern "C" */
#endif
/* NITRO_ARM7_HW_MMAP_GLOBAL_H_ */
#endif
