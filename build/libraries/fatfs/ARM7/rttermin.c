/*
* rttermin.c - Portable portion of terminal IO routines
*  
*   EBS - ERTFS
*  
*   Copyright EBS Inc. 1987-2003
*   All rights reserved.
*   This code may not be redistributed in source or linkable object form
*   without the consent of its author.
*
*
*    Module description:
*        This file contains terminal IO routines used by the sample programs
*        and by routines that print diagnostics. 
*
*
* The rest of the routines in this module are portable with the possible
* exception of these two routines:
* 
* void rtfs_print_format_dir
* void rtfs_print_format_stat
*
* These routines may require porting if sprintf() is not available to you.
* They are called only by the test shell prgram (tstsh.c) and are used to create attractive 
* formatted output for the DIR and STAT commands. They rely on sprintf to format the output 
*  to provide a system specific console output routine. A define is provided
* in this file named SYS_SUPPORTS_SPRINTF, if this is set to one the routines format the output 
* using sprintf, otherwise they print a fixd string. If sprintf is not available to you set 
* SYS_SUPPORTS_SPRINTF to zero. 
*
* 
* The following portable routines are also provided in this file.
*
*   rtfs_print_string_1(int stringid,int flags)
*   rtfs_print_string_2(int stringid,byte *pstr2, int flags)
*
*   These two routines are used to print string values to the console. They are portable,
*   relying on the routine rtfs_port_puts(() to provide a system specific console output routine.
*  If no output is desired define the macros RTFS_PRINT_STRING_1 and RTFS_PRINT_STRING_2 as 
*  no-ops in portterm.h.
*
*   rtfs_print_long_1
*
*   This routine is used to print long integers values to the console. It relies on 
*  rtfs_port_puts(() to provide a system specific console output routine. If no output 
*  is desired define the macro RTFS_PRINT_LONG_1 as a no-op in portterm.h.
*
* 
* rtfs_print_prompt_user() -
*
*   This routine is called when the ERTFS demo programs and critical error handlr routine 
*  requires console input from the user. It takes as input a prompt id (this is a numeric 
*  handle to the prompt strings in the prompts string table prompt_table[]
*   in portstr.c and the address of a buffer where to place the console 
*   input. 
*
*   This routine displays the prompt by calling rtfs_print_one_string() and then
*   calls the target specific routine tm_gets_rtfs() to recieve the console
*   input.
*
*   Note: If in your system console input is not available you may still 
*  use the demo and test applications by modifying this routine so that
*   it returns specific strings to simulate user input. The values to return
*   must be relevant to the value of prompt_id. Here are several prompt_id
*   values that should be responded to if no console input is available.
*
*
*   UPROMPT_CRITERR -  This is the prompt_id argument when a drive IO error
*   occurs. You should return "A" in the return buffer. This will cause the 
*   IO operation to fail and return an error to the API.
*
*   UPROMPT_TSTSH    - If you returned "S" for UPROMPT_RTFSDEM1 then this 
*   prompt will be called after the test shell starts and repeatedly 
*   thereafter. Return strings for this prompt as if you were typing 
*   input to the command shell.
*
*   For example a sequence of strings might fill.
*
*   FILLFILE TEST.DAT MYPATTERN 100
*   CAT TEST.DAT
*   DELETE TEST.BAT
*
*   If you do not wish to use the interactive test programs you need
*   not implement this function.
* If sprintf is not available to you set SYS_SUPPORTS_SPRINTF to zero. 
*/
#define SYS_SUPPORTS_SPRINTF 0
#include <rtfs.h>

#if (SYS_SUPPORTS_SPRINTF)
#include <stdio.h>  /* For sprintf */
#endif


void rtfs_print_one_string(byte *pstr,int flags);
byte *pc_ltoa(dword num, byte *dest);

void rtfs_print_string_1(int stringid,int flags)
{
    rtfs_print_one_string(rtfs_strtab_user_string(stringid),flags);
}

void rtfs_print_string_2(int stringid,byte *pstr2, int flags)
{
    rtfs_print_string_1(stringid,0);
    rtfs_print_one_string(pstr2,flags);
}

void rtfs_print_long_1(dword l,int flags)
{
byte buffer[16];
    rtfs_print_one_string(pc_ltoa(l, buffer),flags);
}

void rtfs_print_prompt_user(int prompt_id, byte *buf)
{
byte inbuff[80];
    rtfs_print_one_string(rtfs_strtab_user_prompt(prompt_id),0);
    rtfs_port_tm_gets(inbuff);
#if (INCLUDE_CS_UNICODE)
    map_ascii_to_unicode(buf, inbuff);
#else
    rtfs_strcpy(buf, inbuff);
#endif
}

/* Porting may be required */
static byte *gotoeos(byte *p) { while(*p) p++; return(p);}
void rtfs_print_format_dir(byte *display_buffer, DSTAT *statobj)
{
#if (SYS_SUPPORTS_SPRINTF)
    byte *dirstr;
    byte *p;
    int year;

    if (statobj->fattribute & AVOLUME)
         dirstr = (byte *)"<VOL>";
    else if (statobj->fattribute & ADIRENT)
         dirstr = (byte *)"<DIR>";
    else
         dirstr = (byte *)"     ";
    
    p = display_buffer;
    *p = 0;

    sprintf((char *)p,"%-8s.", (char *)&(statobj->fname[0]));
    sprintf((char *)gotoeos(p),"%-3s",  (char *)&(statobj->fext[0]));
//    sprintf((char *)gotoeos(p)," %10lu ", statobj->fsize);
    sprintf((char *)gotoeos(p)," %10u ", statobj->fsize);

    sprintf((char *)gotoeos(p),"%5s", dirstr);
    sprintf((char *)gotoeos(p)," %02d",(statobj->fdate >> 5 ) & 0xf); /* Month */
    sprintf((char *)gotoeos(p),"-%02d",(statobj->fdate & 0x1f));      /* Day */
    year = 80 +(statobj->fdate >> 9) & 0xff; /* Year */
    if (year >= 100)
        year -= 100;
    sprintf((char *)gotoeos(p),"-%02d", year); /* Year */
    sprintf((char *)gotoeos(p)," %02d",(statobj->ftime >> 11) & 0x1f);    /* Hour */
    sprintf((char *)gotoeos(p),":%02d",(statobj->ftime >> 5) & 0x3f);    /* Minute */

    /* if lfnmae is null we know that the first two bytes are both 0 */
    if (statobj->lfname[0] || statobj->lfname[1])
    {
        /* For vfat systems display the attributes and the long file name
           seperately. This is a trick since the attribute are ASCII and the
            LFN is UNICODE. If we print seperately we will see them both correctly */ 
        sprintf((char *)gotoeos(p), " -  ");
        rtfs_print_one_string(display_buffer, 0);
        rtfs_print_one_string(statobj->lfname,PRFLG_NL);
    }
    else
        rtfs_print_one_string(display_buffer,PRFLG_NL);
#else /* #if (SYS_SUPPORTS_SPRINTF) */
    rtfs_print_one_string((byte*)"SPRINTF NOT SUPPORTED",PRFLG_NL);
#endif

}

/* Porting may be required */
void rtfs_print_format_stat(byte *display_buffer, ERTFS_STAT *st)
{
#if (SYS_SUPPORTS_SPRINTF)
byte *p;
    p = display_buffer;
    *p = 0;
    sprintf((char *)gotoeos(p),"DRIVENO: %02d SIZE: %7ld", st->st_dev, st->st_size);
    sprintf((char *)gotoeos(p)," DATE:%02d-%02d",(st->st_atime.date >> 5 ) & 0xf,/* Month */(st->st_atime.date & 0x1f)/* Day */);
    sprintf((char *)gotoeos(p),"-%02d",80 +(st->st_atime.date >> 9) & 0xff); /* Year */
    sprintf((char *)gotoeos(p),"  TIME:%02d:%02d\n",(st->st_atime.time >> 11) & 0x1f,/* Hour */(st->st_atime.time >> 5) & 0x3f); /* Minute */
    sprintf((char *)gotoeos(p),"OPTIMAL BLOCK SIZE: %7ld FILE size (BLOCKS): %7ld",st->st_blksize,st->st_blocks);
    rtfs_print_one_string(display_buffer,PRFLG_NL);
#else /* #if (SYS_SUPPORTS_SPRINTF) */
    rtfs_print_one_string((byte*)"SPRINTF NOT SUPPORTED",PRFLG_NL);
#endif

}

void rtfs_print_one_string(byte *pstr,int flags)
{
    
    rtfs_port_puts(CS_OP_FORMAT_OUTPUT(pstr));
    if (flags & PRFLG_NL)
        rtfs_port_puts((byte *)"\n");
    if (flags & PRFLG_CR)
        rtfs_port_puts((byte *)"\r");
}
/* Portable */
byte *pc_ltoa(dword num, byte *dest)      /*__fn__*/
{
byte buffer[33]; /* MAXINT can have 32 digits max, base 2 */
int digit;
byte *olddest = dest;
byte * p;

    p = &(buffer[32]);  

    *p = '\0';

    /* Convert num to a string going from dest[31] backwards */ 
    /* Nasty little ItoA algorithm */
    do
    {
        digit = (int) (num % 10);

        *(--p) =
          (byte)(digit<10 ? (byte)(digit + '0') : (byte)((digit-10) + 'a'));
        num /= 10;
    }
    while (num);

    /* Now put the converted string at the beginning of the buffer */
    while((*dest++=*p++)!='\0');
    return (olddest);
}

