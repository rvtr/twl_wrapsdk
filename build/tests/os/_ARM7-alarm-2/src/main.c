/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - _ARM7_alarm-2
  File:     main.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  Revision 1.8  2006/01/18 02:11:29  kitase_hirotake
  do-indent

  Revision 1.7  2005/11/21 10:56:43  kitase_hirotake
  SVC_WaitVBlankIntr �� OS_WaitVBlankIntr �ɕύX

  Revision 1.6  2005/02/28 05:26:20  yosizaki
  do-indent.

  Revision 1.5  2004/04/15 06:51:12  yada
  only arrange comment

  Revision 1.4  2004/03/08 08:31:58  yada
  fix top comment

  Revision 1.3  2004/02/24 00:16:26  yada
  (none)

  Revision 1.2  2004/02/23 01:00:32  yada
  (none)

  Revision 1.1  2004/02/20 05:16:15  yada
  firstRelease

  Revision 1.5  2004/02/10 05:47:46  yasu
  delete macro CODE32

  Revision 1.4  2004/02/09 11:54:29  yasu
  include code32.h

  Revision 1.3  2004/02/05 07:09:03  yasu
  change SDK prefix iris -> nitro

  Revision 1.2  2004/02/04 07:37:45  yada
  OS_GetSystemClock(), OS_SetSystemClock()��
  OS_GetTime(),OS_SetTime() �֖��̕ύX�����֘A�ŏC��

  Revision 1.1  2004/02/03 12:03:20  yada
  firstRelease

  Revision 1.1  2004/02/03 11:25:17  yada
  firstRelease

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

void    alarmCallback(void *arg);
void    alarmDisp(u32 arg);

void    alarmCallback2(void *arg);
void    alarmDisp2(u32 arg);

void    VBlankIntr(void);


static OSAlarm alarm;
static OSAlarm alarm2;

static BOOL called = FALSE;
static BOOL alarm2_sw = FALSE;

static int pushCounter = 0;

static u16 keyData;


#define  ALARM_COUNT     0x20000
#define  ALARM_COUNT_P1  0x10000
#define  ALARM_COUNT_P2  0x400

#define  TEST_DATA       0x12345678
#define  TEST_DATA2      0x9abcdef0

int     count;

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
#ifdef SDK_DEBUGGER_ARM
    OS_Printf("ARM7 before OS_Init.\n");
#endif

    OS_Init();

    // ���̂Q�� ARM7�ł� os_init() �ł��
    // ---- �V�X�e���N���b�N������
    //OS_InitSystemClock();
    // ---- �A���[���V�X�e��������
    //OS_InitAlarm();


    //---- V�u�����N�ݒ�
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    OS_Printf("ARM7 starts.\n");

    while (1)
    {
        OS_WaitVBlankIntr();

        keyData = PAD_Read();

        //---- PUSH A to start alarm
        if (keyData & PAD_BUTTON_A && !called)
        {
            pushCounter++;
            called = TRUE;

            //---- setting alarm
            OS_CreateAlarm(&alarm);
            OS_SetAlarm(&alarm, ALARM_COUNT, &alarmCallback, (void *)TEST_DATA);

            OS_Printf("set alarm\n");
        }


        //---- PUSH UP to start periodic alarm
        if (1 /*keyData & PAD_KEY_UP */  && !alarm2_sw)
        {
            alarm2_sw = TRUE;

            //---- setting periodic alarm
            OS_CreateAlarm(&alarm2);
//            OS_SetPeriodicAlarm(&alarm2, OS_GetTime() + ALARM_COUNT_P1, ALARM_COUNT_P2,
//                                &alarmCallback2, (void *)TEST_DATA2);

            OS_Printf("set periodic alarm\n");
        }

        //---- PUSH DOWN to stop periodic alarm
        if (keyData & PAD_KEY_DOWN && alarm2_sw)
        {
            alarm2_sw = FALSE;

            OS_CancelAlarm(&alarm2);
        }
    }
}

//----------------------------------------------------------------
//  Alarm callback
//      IRQ �X�^�b�N���n��Ȃ̂ŁA�X�^�b�N��ς��č�Ƃ��s���Ă��܂��B
//      �G�~�����[�^�ł͂����� OS_Printf() ���Ă����v�ł����A
//      IS-DEBUGGER �ł̓X�^�b�N��ꂵ�܂��B
//
//
#include  <nitro/code32.h>
asm void alarmCallback( void* arg )
{
    ldconst   r12, #0x3800400
    stmfd     r12!, {sp, lr}
    mov       sp, r12

    bl        alarmDisp

    ldmfd     sp!, {r12, lr}
    mov       sp, r12
    bx        lr
}
#include  <nitro/codereset.h>


void alarmDisp(u32 arg)
{
    OS_Printf(">>> called alarmCallback. arg=%x PUSH=%x\n", arg, pushCounter);
    //OS_Printf( ">>> sp=%x\n", OSi_GetCurrentStackPointer() );
    called = FALSE;
}


//----------------------------------------------------------------
//  Alarm callback for periodic alarm
//      IRQ �X�^�b�N���n��Ȃ̂ŁA�X�^�b�N��ς��č�Ƃ��s���Ă��܂��B
//      �G�~�����[�^�ł͂����� OS_Printf() ���Ă����v�ł����A
//      IS-DEBUGGER �ł̓X�^�b�N��ꂵ�܂��B
//
//
#include  <nitro/code32.h>
asm void alarmCallback2( void* arg )
{
    ldconst   r12, #0x3800400
    stmfd     r12!, {sp, lr}
    mov       sp, r12

    bl        alarmDisp2

    ldmfd     sp!, {r12, lr}
    mov       sp, r12
    bx        lr
}
#include  <nitro/codereset.h>


void alarmDisp2(u32 arg)
{
    static int cnt = 0;
    if (cnt++ > 10)
    {
        OS_Panic("END\n");
    }

//    OS_Printf(">>> called alarmCallback2. arg=%x SYSCLOCK=%x\n", arg, OS_GetTime());
//    OS_Printf( ">>> sp=%x\n", OSi_GetCurrentStackPointer() );
}


//----------------------------------------------------------------
//  VBlank interrupt handler
void VBlankIntr(void)
{
    //---- ���荞�݃`�F�b�N�t���O
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*====== End of main.c ======*/
