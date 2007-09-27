/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - mcu - mcu-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl_sp.h>
#include <twl/mcu.h>


static void dump(const char *str, void *ptr, u32 length)
{
    u8 *rdata = (u8*)ptr;
    int i;
    OS_TPrintf("\n[%s] (%d bytes):\n\t", str, length);
    for (i = 0; i < length; i++) {
        OS_TPrintf("%02X", *rdata++);
        if ((i & 0xF) == 0xF) OS_TPrintf("\n\t");
        else    OS_TPrintf(" ");
    }
    OS_TPrintf("\n");
}

static void test_normal(void)
{
    u8 state;
    OS_TPrintf("\n\n[%s]\n\n", __func__);

    state = (u8)MCU_IsResetRequest();
    OS_TPrintf("MCU_IsResetRequest()            : %s\n", state ? "TRUE" : "FALSE");
    OS_TPrintf("\n");

    state = (u8)MCU_GetWifiLedStatus();
    OS_TPrintf("MCU_GetWifiLedStatus()          : %s\n", state ? "TRUE" : "FALSE");
    state = (u8)MCU_SetWifiLedStatus(TRUE);
    OS_TPrintf("MCU_SetWifiLedStatus(TRUE)      : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetWifiLedStatus();
    OS_TPrintf("MCU_GetWifiLedStatus()          : %s\n", state ? "TRUE" : "FALSE");
    SVC_WaitByLoop(0x100000);
    state = (u8)MCU_SetWifiLedStatus(FALSE);
    OS_TPrintf("MCU_SetWifiLedStatus(FALSE)     : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetWifiLedStatus();
    OS_TPrintf("MCU_GetWifiLedStatus()          : %s\n", state ? "TRUE" : "FALSE");
    OS_TPrintf("\n");

    state = (u8)MCU_GetCameraLedPattern();
    OS_TPrintf("MCU_GetCameraLedPattern()       : %s\n", state ? "Blink" : "None");
    state = (u8)MCU_SetCameraLedPattern(MCU_CAMLED_PATTERN_BLINK);
    OS_TPrintf("MCU_SetCameraLedPattern(BLINK)  : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetCameraLedPattern();
    OS_TPrintf("MCU_GetCameraLedPattern()       : %s\n", state ? "Blink" : "None");
    state = (u8)MCU_SetCameraLedPattern(MCU_CAMLED_PATTERN_NONE);
    OS_TPrintf("MCU_SetCameraLedPattern(NONE)   : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetCameraLedPattern();
    OS_TPrintf("MCU_GetCameraLedPattern()       : %s\n", state ? "Blink" : "None");
    OS_TPrintf("\n");

    state = (u8)MCU_GetCardLedStatus(1);
    OS_TPrintf("MCU_GetCardLedStatus(1)         : %s\n", state == 0 ? "Auto" : (state == 1 ? "Off" : "On"));
    state = (u8)MCU_SetCardLedStatus(1, MCU_CARDLED_STATUS_ON);
    OS_TPrintf("MCU_SetCardLedStatus(1, ON)     : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetCardLedStatus(1);
    OS_TPrintf("MCU_GetCardLedStatus(1)         : %s\n", state == 0 ? "Auto" : (state == 1 ? "Off" : "On"));
    SVC_WaitByLoop(0x100000);
    state = (u8)MCU_SetCardLedStatus(1, MCU_CARDLED_STATUS_AUTO);
    OS_TPrintf("MCU_SetCardLedStatus(1, AUTO)   : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetCardLedStatus(1);
    OS_TPrintf("MCU_GetCardLedStatus(1)         : %s\n", state == 0 ? "Auto" : (state == 1 ? "Off" : "On"));
    OS_TPrintf("\n");

    state = (u8)MCU_GetCardLedStatus(2);
    OS_TPrintf("MCU_GetCardLedStatus(2)         : %s\n", state == 0 ? "Auto" : (state == 1 ? "Off" : "On"));
    state = (u8)MCU_SetCardLedStatus(2, MCU_CARDLED_STATUS_ON);
    OS_TPrintf("MCU_SetCardLedStatus(2, ON)     : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetCardLedStatus(2);
    OS_TPrintf("MCU_GetCardLedStatus(2)         : %s\n", state == 0 ? "Auto" : (state == 1 ? "Off" : "On"));
    SVC_WaitByLoop(0x100000);
    state = (u8)MCU_SetCardLedStatus(2, MCU_CARDLED_STATUS_AUTO);
    OS_TPrintf("MCU_SetCardLedStatus(2, AUTO)   : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetCardLedStatus(2);
    OS_TPrintf("MCU_GetCardLedStatus(2)         : %s\n", state == 0 ? "Auto" : (state == 1 ? "Off" : "On"));
    OS_TPrintf("\n");

    state = (u8)MCU_GetVolume();
    OS_TPrintf("MCU_GetVolume()                 : %d\n", state);
    state = (u8)MCU_SetVolume(15);
    OS_TPrintf("MCU_SetVolume(15)               : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetVolume();
    OS_TPrintf("MCU_GetVolume()                 : %d\n", state);
    state = (u8)MCU_SetVolume(31);
    OS_TPrintf("MCU_SetVolume(31)               : %s\n", state ? "SUCCESS" : "FAILED");
    state = (u8)MCU_GetVolume();
    OS_TPrintf("MCU_GetVolume()                 : %d\n", state);
    OS_TPrintf("\n");

    state = (u8)MCU_GetVersion();
    OS_TPrintf("MCU_GetVersion()                : %d\n", state);
    state = (u8)MCU_GetRevision();
    OS_TPrintf("MCU_GetRevision()               : %d\n", state);
    OS_TPrintf("\n");
}

/*
    slowRateTableは、I2Cライブラリの一部で、パラメータ調整用に
    一時的にグローバル変数にしている。
    最終的にstatic変数にするか、残すなら名前を変える必要がある。
*/
extern s32 slowRateTable[];

static void test_tuning(void)
{
    u8 rdata[10];
    u8 wdata[10];
    s32 *pInterval = &slowRateTable[I2C_SLAVE_MICRO_CONTROLLER];
    const u8 offset = 0x0;
    const u32 nums = sizeof(rdata);
    const int times = 200;
    s32 max = 0;
    int i, j, k;

    OS_TPrintf("\n\n[%s]\n\n", __func__);

    for (j = 0; j < nums; j++)
    {
        rdata[j] = 0;
        wdata[j] = (u8)(nums + j);
    }

    // rough tuning
    for (i = 4; i < 30; i++)
    {
        *pInterval = 1 << i;
        SVC_WaitByLoop(0x200);

        for (j = 0; j < nums; j++)
        {
            wdata[j]++;
        }

        if (MCU_SetFreeRegisters(offset, wdata, nums) && MCU_GetFreeRegisters(offset, rdata, nums))
        {
            for (j = 0; j < nums; j++)
            {
                if (wdata[j] != rdata[j])
                {
                    break;
                }
            }
            if (j == nums)
            {
                OS_TPrintf("Success: 0x%X (%d)\n", *pInterval, *pInterval);
                break;
            }
        }
    }

    // fine tuning
    for (k = 0; k < times; k++)
    {
        while (1)
        {
            (*pInterval)--;

            for (j = 0; j < nums; j++)
            {
                wdata[j]++;
            }

            if (!MCU_SetFreeRegisters(offset, wdata, nums) || !MCU_GetFreeRegisters(offset, rdata, nums))
            {
                break;
            }
            for (j = 0; j < nums; j++)
            {
                if (wdata[j] != rdata[j])
                {
                    break;
                }
            }
            if (j != nums)
            {
                break;
            }
        }
        OS_TPrintf("Failed:  0x%X (%d), ", *pInterval, *pInterval);
        SVC_WaitByLoop(0x100);

        while (1)
        {
            (*pInterval)++;

            for (j = 0; j < nums; j++)
            {
                wdata[j] = (u8)(nums + j + k);
            }

            if (MCU_SetFreeRegisters(offset, wdata, nums) && MCU_GetFreeRegisters(offset, rdata, nums))
            {
                for (j = 0; j < nums; j++)
                {
                    if (wdata[j] != rdata[j])
                    {
                        break;
                    }
                }
                if (j == nums)
                {
                    break;
                }
            }
        }
        OS_TPrintf("Success: 0x%X (%d)\n", *pInterval, *pInterval);
        if (max < *pInterval)
        {
            max = *pInterval;
        }
    }
    *pInterval = max;
    {
        OSTick begin;
        OS_InitTick();
        OS_TPrintf("\n\nResult: interval = 0x%08X (%d) ", max, max);
        begin = OS_GetTick();
        SVC_WaitByLoop(max);
        OS_TPrintf("== %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));
    }

    // aging
    OS_TPrintf("\naging...");
    for (k = 0; k < times; k++)
    {
        for (j = 0; j < nums; j++)
        {
            wdata[j]--;
        }
        if (!MCU_SetFreeRegisters(offset, wdata, nums) || !MCU_GetFreeRegisters(offset, rdata, nums))
        {
            (*pInterval)++;
            k = 0;
            SVC_WaitByLoop(0x100);
            continue;
        }
        for (j = 0; j < nums; j++)
        {
            if (wdata[j] != rdata[j])
            {
                (*pInterval)++;
                k = 0;
                SVC_WaitByLoop(0x100);
                continue;
            }
        }
    }
    max = *pInterval;
    {
        OSTick begin;
        OS_InitTick();
        OS_TPrintf("\n\nResult: interval = 0x%08X (%d) ", max, max);
        begin = OS_GetTick();
        SVC_WaitByLoop(max);
        OS_TPrintf("== %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));
    }
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlSpMain()
{
    OS_Init();

    OS_Printf("ARM7 starts.\n");

    test_normal();

    // tune I2C interval
    test_tuning();

    // done
    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
