/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* rtutbyte.c - Character set independent string manipulation routines */

#include <twl.h>
#include <rtfs.h>
#include <rtfs_target_os.h>

/* Byte oriented */
byte *pc_strchr(byte *string, byte ch)
{
    for(;string && *string!=0; string++)
    {
        if(ch == *string) return(string);
    }
    return(0);
}
BOOLEAN _illegal_alias_char(byte ch)        /*__fn__*/
{
    if (pc_strchr(rtfs_strtab_user_string(USTRING_SYS_BADALIAS), ch))
        return(TRUE);
    else
        return(FALSE);
}


/* Byte oriented */
int rtfs_strcpy(byte * targ, byte * src)   /*__fn__*/
{
    int loop_cnt=0;
    do
    {
        targ[loop_cnt] = src[loop_cnt];
    } while(src[loop_cnt++]);
    return loop_cnt;
}

/* Byte oriented */
/* compares 2 strings; returns 0 if they match */
int rtfs_strcmp(byte * s1, byte * s2)        /*__fn__*/
{
    int index=0;
    while (s1[index] && s2[index] && s1[index] == s2[index])
    {
        index++;
    }
    if (!s1[index] && !s2[index])        return 0;
    if (s1[index] < s2[index])           return -1;
    else                                 return 1;
}

/* Byte oriented */
int rtfs_strlen(byte * string)   /*__fn__*/
{
    int len=0;
    while (string[len] != 0) len++;
    return len;
}

/* Byte oriented string functions character set independent */
/****************************************************************************
COPYBUF  - Copy one buffer to another
Description
Essentially strncpy. Copy size BYTES from from to to.
Returns
Nothing
****************************************************************************/
void copybuff(void *vto, void *vfrom, int size)                                 /* __fn__*/
{
    OSAPI_CPUCOPY8( vfrom, vto, size);
/*    byte *to = (byte *) vto;
    byte *from = (byte *) vfrom;
    while (size--)
        *to++ = *from++;*/
}

/******************************************************************************
PC_CPPAD  - Copy one buffer to another and right fill with spaces
Description
Copy up to size characters from from to to. If less than size
characters are transferred before reaching \0 fill to with SPACE
characters until its length reaches size. 

  Note: to is NOT ! Null terminated.
  
    ASCII character function only. Unicode not required
    
      Returns
      Nothing
      
*****************************************************************************/
/* Byte oriented */
void pc_cppad(byte *to, byte *from, int size)                                       /* __fn__*/
{
    rtfs_memset(to, ' ', size);
    while (size--)
        if (*from)
            *to++ = *from++;
}

/* ******************************************************************** */

/* Byte oriented */
void rtfs_memset(void *pv, byte b, int n)                      /*__fn__*/
{ byte *p; p = (byte *) pv; while(n--) {*p++=b;} }

/* Byte oriented see rtfs_cs_strcat for char oriented */
byte * rtfs_strcat(byte * targ, byte * src)   /*__fn__*/
{
    int ret_val;
    ret_val = rtfs_strlen(targ);
    rtfs_strcpy(targ + ret_val, src);
    return targ;
}

