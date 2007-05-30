/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 2002
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
* UTILASCI.C - Contains ASCII string manipulation and character 
*  conversion routines 
*/

#include <rtfs.h>

#if (INCLUDE_CS_ASCII)

byte *ascii_goto_eos(byte *p)
{
    while (*p) p++;
    return(p);
}

void pc_byte2upper(byte *to, byte *from)
{
byte c;
    c = *from;
    if  ((c >= 'a') && (c <= 'z'))
        c = (byte) ('A' + c - 'a');
    *to = c;
}

void pc_ascii_strn2upper(byte *to, byte *from, int n)
{
    int i;
    for (i = 0; i < n; i++,to++, from++)
        pc_byte2upper(to, from);
}

void pc_ascii_str2upper(byte *to, byte *from)
{
        while(*from)
            pc_byte2upper(to++, from++);
        *to = '\0';
}

void pc_str2upper(byte *to, byte *from)
{
    pc_ascii_str2upper(to, from);
}

/***************************************************************************
        PC_MFILE  - Build a file spec (xxx.yyy) from a file name and extension

 Description
        Fill in to with a concatenation of file and ext. File and ext are
        not assumed to be null terminated but must be blank filled to [8,3]
        chars respectively. 'to' will be a null terminated string file.ext.

        ASCII character function only. Unicode not required

 Returns
        A pointer to 'to'.

****************************************************************************/
byte *pc_ascii_mfile(byte *to, byte *filename, byte *ext)
{
        byte *p;
        int i;
        byte *retval = to;

        p = filename;
        i = 0;
        while(*p)
        {
                if (*p == ' ')
                        break;
                else
                {
                        *to++ = *p++;
                        i++;
                }
                if (i == 8)
                        break;
        }
        if (p != filename)
        {
                p = ext;
                if (*p && *p != ' ')
                    *to++ = '.';
                i = 0;
                while(p && *p)
                {
                        if (*p == ' ')
                                break;
                        else
                        {
                                *to++ = *p++;
                                i++;
                        }
                        if (i == 3)
                                break;
                }
        }
         *to = '\0';
        return (retval);
}
/* Version of MFILE that converts path from byte orientation to 
   native char set before returning. pc_mfile and pc_cs_mfile are
   the same for ascii */    
byte *pc_cs_mfile(byte *to, byte *filename, byte *ext)
{
    return(pc_ascii_mfile(to, filename, ext));
}

byte * rtfs_cs_strcat(byte * targ, byte * src)    /*__fn__*/
{
    return(rtfs_strcat(targ, src));
}

int rtfs_cs_strcmp(byte * s1, byte * s2)
{
    return (rtfs_strcmp(s1, s2));
}

int rtfs_cs_strcpy(byte * targ, byte * src)
{
    return (rtfs_strcpy(targ, src));
}

/* return number of ascii chars in a string */
int rtfs_cs_strlen(byte * string)   /*__fn__*/
{
    return (rtfs_strlen(string));
}
/* return number of bytes in an ascii character string */
int rtfs_cs_strlen_bytes(byte * string)   /*__fn__*/
{
    return (rtfs_strlen(string));
}

int ascii_ascii_index(byte *p, byte base)
{
byte c;
    pc_byte2upper(&c, p);
    return((int) (c - base));
}

int ascii_compare_nc(byte *p1, byte *p2)
{
byte c,d;
    if (*p1 == *p2)
        return(1);
    else
    {
        pc_byte2upper(&c, p1);
        pc_byte2upper(&d, p2);
        return(c == d);
    }
}

/***************************************************************************
PC_FILEPARSE -  Parse a file xxx.yyy into filename/pathname

  Description
  Take a file named XXX.YY and return SPACE padded NULL terminated 
  filename [XXX       ] and fileext [YY ] components. If the name or ext are
  less than [8,3] characters the name/ext is space filled and null termed.
  If the name/ext is greater  than [8,3] the name/ext is truncated. '.'
  is used to seperate file from ext, the special cases of . and .. are
  also handled.
  Returns
  Returns TRUE
  
    ****************************************************************************/
    /* UNICODE - Called by pc_malias not by others if vfat - Okay as ascii */
    /* UNICODE - pc_enum usage is probably incorrect */
    /* Take a string xxx[.yy] and put it into filename and fileext */
    /* Note: add a check legal later */
BOOLEAN pc_ascii_fileparse(byte *filename, byte *fileext, byte *p)                  /* __fn__*/
{
    int i;
    
    /* Defaults */
    rtfs_memset(filename, ' ', 8);
    filename[8] = '\0';
    rtfs_memset(fileext, ' ', 3);
    fileext[3] = '\0';
    
    /* Special cases of . and .. */
    if (*p == '.')
    {
        *filename = '.';
        if (*(p+1) == '.')
        {
            *(++filename) = '.';
            return (TRUE);
        }
        else if (*(p + 1) == '\0')
            return (TRUE);
        else
            return (FALSE);
    }
    
    i = 0;
    while (*p) 
    {
        if (*p == '.')
        {
            p++;
            break;
        }
        else
            if (i++ < 8)
                *filename++ = *p;
            p++;
    }
    
    i = 0;
    while (*p)
    {
        if (i++ < 3)
            *fileext++ = *p;
        p++;
    }
    return (TRUE);
}

/***************************************************************************
    PC_VALID_SFN - See if filename is a valid short file name

Description
    Determines validity of a short file name based on the following criteria:
        - the file name must be between 0 and 8 characters
        - the file extension must be between 0 and 3 characters
        - the file name must not begin with a period
        - it must not be a reserved DOS file name
        - it must not contain any characters that are illegal within sfn's  
Returns
    TRUE if filename is a valid sfn, FALSE otherwise
****************************************************************************/

BOOLEAN pc_valid_sfn(byte *filename)
{
    int len,period_count,ext_start;
    byte name_part[9];

    BOOLEAN badchar;
    if(name_is_reserved(filename)) return(FALSE);
    name_part[0] = 0;
    for(len=0,badchar=FALSE,period_count=0,ext_start=0; filename[len]!=0; len++)
    {
        if(_illegal_alias_char(filename[len])) badchar = TRUE;
        if(filename[len] == '.') 
        {
            ext_start = len+1;
            period_count++;
        }
        else
        {
            if (!ext_start && len < 8)
            {
                name_part[len] = filename[len];
                name_part[len+1] = 0;
            }
        }
    }

    /* check if the file name part contains a reserved name */
    if(name_is_reserved(name_part)) return(FALSE);

    if( (filename[0] == ' ') ||                  /* 1st char is a space */
        (len == 0) ||                            /* no name */
        badchar ||                               /* contains illegal chars */
        (period_count > 1) ||                    /* contains more than one extension */
        ((len-ext_start)>3 && period_count>0) || /* extension is longer than 3 chars */
        (period_count==0 && len > 8) ||          /* name is longer than 8 chars */
        (ext_start > 9) ||                       /* name is longer than 8 chars */
        (ext_start==1) ) return(FALSE);          /* no name; 1st char is a period */
    
    return(TRUE);
}

#if (VFAT) /* Small piece of compile time VFAT vs's NONVFAT code  */

/***************************************************************************
    PC_VALID_LFN - See if filename is a valid long file name

Description
    Determines validity of a long file name based on the following criteria:
        - the file must be between 0 and 256 characters in length
        - it must not be a reserved DOS file name
        - it must not contain any characters that are illegal within lfn's  
Returns
    TRUE if filename is a valid lfn, FALSE otherwise
*****************************************************************************/

BOOLEAN validate_filename(byte * filename, byte * ext)
{
    int len,n;
    byte name_part[9];

    RTFS_ARGSUSED_PVOID((void *) ext);

    name_part[0] = 0;
    for(n=0,len=0; filename[n]!=0; len++,n++)
    {
        if(_illegal_lfn_char(filename[n]))
            return(FALSE);
        else
        {
            if (len < 5)    /* handles lpt1, aux, con etc */
            {
                if(filename[len] == '.') 
                {
                    name_part[len] = 0;
                    if(name_is_reserved(name_part)) return(FALSE);
                }
                else
                {
                    name_part[len] = filename[len];
                    if (filename[len+1] == 0)
                    {
                        name_part[len+1] = 0;
                        if(name_is_reserved(name_part)) return(FALSE);
                    }
                }
            }
        }
    }
    if( (len == 0) || (len > 255) ) return(FALSE);

    return(TRUE);   
}


BOOLEAN pc_cs_malias(byte *alias, byte *input_file, int try)
{
    int n,i,s;
    byte filename[9],fileext[4];

    /* Check if invalid short file name.Case is ignored. If ignored we fix it later !  */
    if ((try == -1) && !pc_valid_sfn((byte *)input_file))
        return(FALSE);

    /* Process the ASCII alias name */
    while(*input_file=='.' || *input_file==' ') input_file++;

    /* find extension start */
    for(n=0,i=0; input_file[n]!=0; n++) /* i holds the position right */
    {                                   /* after the last period      */
        if(input_file[n]=='.') i=n+1;
    }
    
    if(i>0 && input_file[i]!=0){
    /* copy extension to fileext[] */
    for(n=i,s=0; input_file[n]!=0 && s<3; n++)
    {
        if(input_file[n]!=' ')
        {
        if(_illegal_alias_char(input_file[n]))
            {
                fileext[s++] = '_';
            }
            else
                fileext[s++] = input_file[n];
        }
    }
    fileext[s]=0;} else { i=512; fileext[0]=0; } /* null terminate */

    /* copy file name to filename[], filtering out spaces, periods and
        replacing characters illegal in alias names with '_' */
    for(n=0,s=0; n<i && input_file[n]!=0 && s<8; n++)
    {
        if(input_file[n]!=' ' && input_file[n]!='.')
        {
            if(_illegal_alias_char(input_file[n]) || input_file[n]>127)
            {
                filename[s++] = '_';
            }
            else
                filename[s++] = input_file[n];
        }
    }
    for(;s<8;s++) /* pad with spaces */
    {
        filename[s] = ' ';
    }   
    filename[8]=0; /* null terminate filename[] */

    pc_ascii_str2upper(filename,filename);
    pc_ascii_str2upper(fileext,fileext);

    if (try != -1)
    {
        /* append (TEXT[])i to filename[] */
        for(n=7,s=try; s>0 && n>0; s/=10,n--) 
        {
             filename[n] = (byte)(((byte)s%10)+'0');
        }
        if(n==0 && s>0)
             return(FALSE);
        else
             filename[n]='~'; 
    }
     /* copy filename[] to alias[], filtering out spaces */
    for(n=0,s=0; s<8; s++)
    {
        if(filename[s]!=' ')
            alias[n++]=filename[s];
    }
    if(fileext[0] != 0) 
    { 
        alias[n++]='.'; /* insert separating period */
        /* copy fileext[] to alias[] */ 
        for(s=0; fileext[s]!=0; s++,n++)
        {
            alias[n]=fileext[s];
        }   
     }
     alias[n]=0; /* null terminate alias[] */

    return(TRUE);
}
#endif 

#if (!VFAT) /* Small piece of compile time VFAT vs's NONVFAT code  */




BOOLEAN validate_8_3_name(byte * name,int len) /*__fn__*/
{
    int i;
    int last;

    last = len-1;
    for (i = 0; i < len; i++)
    {
        /* If we hit a space make sure the rest of the string is spaces */
        if (name[i] == ' ')
        {
            for (; i < len; i++)
            {
                if (name[i] != ' ')
                    return(FALSE);
            }
            break;
        }
    }
    return(TRUE);
}

BOOLEAN validate_filename(byte * name, byte * ext)
{
byte sfn_buffer[14]; /* Only uses 13 but keep declarations even */
    if (!( validate_8_3_name(name,8) && validate_8_3_name(ext,3)) )
        return(FALSE);      
    pc_ascii_mfile(sfn_buffer, name, ext);
    return (pc_valid_sfn(sfn_buffer));
}

BOOLEAN pc_patcmp_8(byte *p, byte *pattern, BOOLEAN dowildcard)  /* __fn__*/
{
int size = 8;
    /* Kludge. never match a deleted file */
    if (*p == PCDELETE)
       return (FALSE);
    else if (*pattern == PCDELETE)  /* But E5 in the Pattern matches 0x5 */
    {
        if (*p == 0x5)
        {
            size -= 1;
            p++;
            pattern++;
        }
        else
           return (FALSE);
   }

    while (size--)
    {
        if(dowildcard)
        {
            if (*pattern == '*')    /* '*' matches the rest of the name */
                return (TRUE);
            if (*pattern != '?' && !ascii_compare_nc(pattern,p))
                  return (FALSE);
        }
        else
        {
            if (!ascii_compare_nc(pattern,p))
                 return (FALSE);
        }
        p++;
        pattern++;
    }
    return (TRUE);
}

BOOLEAN pc_patcmp_3(byte *p, byte *pattern, BOOLEAN dowildcard)  /* __fn__*/
{
int size = 3;

    while (size--)
    {
        if(dowildcard)
        {
            if (*pattern == '*')    /* '*' matches the rest of the name */
                return (TRUE);
            if (*pattern != '?' && !ascii_compare_nc(pattern,p))
                 return (FALSE);
        }
        else
        {
            if (!ascii_compare_nc(pattern,p))
                 return (FALSE);
        }
        p++;
        pattern++;
    }
    return (TRUE);
}
#endif /* VFAT */

#endif

