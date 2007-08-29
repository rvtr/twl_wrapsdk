/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MCU - demos - mcu-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>

//================================================================================
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

extern s32 I2CSlowRateTable[];

static void test_normal(void)
{
    OS_TPrintf("\n\n[%s]\n\n", __func__);

    OS_TPrintf("MCU_IsResetRequest: %s\n", MCU_IsResetRequest() ? "TRUE" : "FALSE");
    OS_TPrintf("\n");

    OS_TPrintf("MCU_IsWifi        : %s\n", MCU_IsWifi() ? "TRUE" : "FALSE");
    OS_TPrintf("MCU_SetWifi(TRUE) : %s\n", MCU_SetWifi(TRUE) ? "SUCCESS" : "FAILED");
    OS_TPrintf("MCU_IsWifi        : %s\n", MCU_IsWifi() ? "TRUE" : "FALSE");
    SVC_WaitByLoop(0x100000);
    OS_TPrintf("MCU_SetWifi(FALSE): %s\n", MCU_SetWifi(FALSE) ? "SUCCESS" : "FAILED");
    OS_TPrintf("MCU_IsWifi        : %s\n", MCU_IsWifi() ? "TRUE" : "FALSE");
    OS_TPrintf("\n");

    OS_TPrintf("MCU_GetCameraPattern                          : %s\n", MCU_GetCameraPattern() ? "Blink" : "None");
    OS_TPrintf("MCU_SetCameraPattern(MCU_CAMERA_PATTERN_BLINK): %s\n", MCU_SetCameraPattern(MCU_CAMERA_PATTERN_BLINK) ? "SUCCESS" : "FAILED");
    OS_TPrintf("MCU_GetCameraPattern                          : %s\n", MCU_GetCameraPattern() ? "Blink" : "None");
    OS_TPrintf("MCU_SetCameraPattern(MCU_CAMERA_PATTERN_NONE) : %s\n", MCU_SetCameraPattern(MCU_CAMERA_PATTERN_NONE) ? "SUCCESS" : "FAILED");
    OS_TPrintf("MCU_GetCameraPattern                          : %s\n", MCU_GetCameraPattern() ? "Blink" : "None");
    OS_TPrintf("\n");

    OS_TPrintf("MCU_GetVolume    : %d\n", MCU_GetVolume());
    OS_TPrintf("MCU_SetVolume(15): %s\n", MCU_SetVolume(15) ? "SUCCESS" : "FAILED");
    OS_TPrintf("MCU_GetVolume    : %d\n", MCU_GetVolume());
    OS_TPrintf("MCU_SetVolume(31): %s\n", MCU_SetVolume(31) ? "SUCCESS" : "FAILED");
    OS_TPrintf("MCU_GetVolume    : %d\n", MCU_GetVolume());
    OS_TPrintf("\n");

    OS_TPrintf("MCU_GetVersion : %d\n", MCU_GetVersion());
    OS_TPrintf("MCU_GetRevision: %d\n", MCU_GetRevision());
    OS_TPrintf("\n");
}

static void test_tuning(void)
{
    u8 rdata[10];
    u8 wdata[10];
    s32 *pInterval = &I2CSlowRateTable[I2C_SLAVE_MICRO_CONTROLLER];
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
    for (i = 6; i < 30; i++)
    {
        *pInterval = 1 << i;
        SVC_WaitByLoop(0x200);

        for (j = 0; j < nums; j++)
        {
            wdata[j]++;
        }

        if (MCU_GetFreeRegisters(offset, wdata, nums) && MCU_SetFreeRegisters(offset, rdata, nums))
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

            if (!MCU_GetFreeRegisters(offset, wdata, nums) || !MCU_SetFreeRegisters(offset, rdata, nums))
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

            if (MCU_GetFreeRegisters(offset, wdata, nums) && MCU_SetFreeRegisters(offset, rdata, nums))
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
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    OS_Init();

    OS_Printf("ARM7 starts.\n");

    test_normal();

    // tune I2C interval
    //test_tuning();

    // done
    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
