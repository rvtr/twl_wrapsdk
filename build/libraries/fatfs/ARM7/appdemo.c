/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren , 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
* appdemo.c - Top level demo code. Initializes ertfs and calls the test shell.
*/
#include <rtfs.h>

void tst_shell(void);

void  rtfs_app_entry(void)                             /* __fn__ */
{

    /* Initialize ertfs  */
    if (!pc_ertfs_init())
    {
        RTFS_PRINT_STRING_1(USTRING_RTFSDEM_01, PRFLG_NL); /* "pc_ertfs_init failed" */
        return;
    }
    tst_shell();
}

