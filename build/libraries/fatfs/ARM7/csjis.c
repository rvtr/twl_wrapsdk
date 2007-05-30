/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 2002
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* JIS.C - Contains Japanese string manipulation and character conversion routines */
/*         See also jistab.c */


#include <rtfs.h>

#if (INCLUDE_CS_JIS)

byte *jis_goto_eos(byte *p)
{
    while (*p)
        p = jis_increment(p);
    return(p);
}

/* Return the length of a JIS character, 1 or 2 */
int jis_char_length(byte *p)
{
    if ((*p >= 0x81 && *p <= 0x9f) || (*p >= 0xe0 && *p <= 0xfc))
        return(2);
    else
        return(1);
}

/* Copy JIS character, 1 or 2 bytes */
int jis_char_copy(byte *to, byte *from)
{
int len;
    len = jis_char_length(from);
    *to++ = *from++;
    if (len == 2)
        *to++ = *from++;
    return(len);
}

/* Advance a pointer to the next JIS character in a string */
byte *jis_increment(byte *p)
{
    return(p + jis_char_length(p));
}

/* return number of jis chars in a string */
/* Return 1 if p1 and p2 are the same */
int jis_compare(byte *p1, byte *p2)
{
    /* If 1st char same and (len is 1 or 2nd char the same */
    return ( (*p1 == *p2) && (jis_char_length(p1)==1 || (*(p1+1) == *(p2+1))) );
}

int jis_ascii_index(byte *p, byte base)
{
byte c;
    pc_byte2upper(&c, p);
    return((int) (c - base));
}

int jis_compare_nc(byte *p1, byte *p2)
{
byte c,d;
    if (jis_compare(p1, p2))
        return(1);
    else if (jis_char_length(p1)==1 && jis_char_length(p2)==1)
    {
        pc_byte2upper(&c, p1);
        pc_byte2upper(&d, p2);
        return(c == d);
    }
    else
        return(0);
}


void pc_ascii_strn2upper(byte *to, byte *from, int n)  /* __fn__*/
{
    int i;
    byte c;
    for (i = 0; i < n; i++)
    {
            if (jis_char_length(from) == 2)
            {
                *to++ = *from++;
                *to++ = *from++;
            }
            else
            {
                c = *from++;
                if  ((c >= 'a') && (c <= 'z'))
                        c = (byte) ('A' + c - 'a');
                *to++ = c;
            }
    }
}

void pc_byte2upper(byte *to, byte *from)  /* __fn__*/
{
byte c;
    c = *from;
    if  ((c >= 'a') && (c <= 'z'))
        c = (byte) ('A' + c - 'a');
    *to = c;
}


void pc_ascii_str2upper(byte *to, byte *from)  /* __fn__*/
{
        byte c;
        while(*from)
        {
            if (jis_char_length(from) == 2)
            {
                *to++ = *from++;
                *to++ = *from++;
            }
            else
            {
                c = *from++;
                if  ((c >= 'a') && (c <= 'z'))
                        c = (byte) ('A' + c - 'a');
                *to++ = c;
            }
        }
        *to = '\0';
}
void pc_str2upper(byte *to, byte *from)  /* __fn__*/
{
    pc_ascii_str2upper(to, from);
}

/* compares 2 strings; returns 0 if they match */
int rtfs_cs_strcmp(byte * s1, byte * s2)        /*__fn__*/
{
    while (*s1 && *s2)
    {
        if (!jis_compare(s1, s2))
            return(1);
        s1 = jis_increment(s1);
        s2 = jis_increment(s2);
    }
    if (!*s1 && !*s2)
        return(0);
    else
        return(1);
}
/* This works because JIS has no ZERO in hi byte of 16 bit jis chars */
byte * rtfs_cs_strcat(byte * targ, byte * src)    /*__fn__*/
{
    /* Call the byte oriented function */
    return(rtfs_strcat(targ, src));
}

/* This works because JIS has no ZERO in hi byte of 16 bit jis chars */
int rtfs_cs_strcpy(byte * targ, byte * src)
{
    /* Call the byte oriented function */
    return (rtfs_strcpy(targ, src));
}

/* return number of jis chars in a string */
int rtfs_cs_strlen(byte * string)   /*__fn__*/
{
int len=0;
   while (*string)
   {
    string = jis_increment(string);
    len++;
   }
   return len;
}
/* return number of bytes in a jis character string */
int rtfs_cs_strlen_bytes(byte * string)   /*__fn__*/
{
byte *s;
int len;
   s = string;
   while (*string)
   {
    string = jis_increment(string);
   }
   len = (int) (s-string);
   return len;
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
byte *pc_ascii_mfile(byte *to, byte *filename, byte *ext)                         /* __fn__*/
{
        byte *p;
        int i,l;
        byte *retval = to;

        p = filename;
        i = 0;
        while(*p)
        {
                l = jis_char_length(p);

                if (*p == ' ')
                        break;
                else
                {
                    if (l == 2)
                    {
                        if (i>6)
                            break;
                        *to++ = *p++;
                    }
                    *to++ = *p++;
                    i += l;
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
                    l = jis_char_length(p);
                    if (*p == ' ')
                        break;
                    else
                    {
                        if (l == 2)
                        {
                            if (i>1)
                                break;
                            *to++ = *p++;
                        }
                        *to++ = *p++;
                        i += l;
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
   the same for jis */
byte *pc_cs_mfile(byte *to, byte *filename, byte *ext)
{
    return(pc_ascii_mfile(to, filename, ext));
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
    /* Take a string xxx[.yy] and put it into filename and fileext */
    /* Note: add a check legal later */
BOOLEAN pc_ascii_fileparse(byte *filename, byte *fileext, byte *p)                  /* __fn__*/
{
    int i;
    int l;

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
        l = jis_char_length(p);
        if (*p == '.')
        {
            p++;
            break;
        }
        else
        {
            if (i + l <= 8)
            {
                jis_char_copy(filename, p);
                filename += l;
                i += l;
            }
            p += l;
        }
    }

    i = 0;
    while (*p)
    {
        l = jis_char_length(p);
        if (i + l <= 3)
        {
            jis_char_copy(fileext, p);
            fileext += l;
            i += l;
        }
        p += l;
    }
    return (TRUE);
}



/* Return TRUE if illegal for an 8 dot 3 file */
static BOOLEAN _illegal_jis_file_char(byte *p, int islfn)
{
byte c;
    if (jis_char_length(p) == 1)
    {
        c = *p;
        /* Test for valid chars. Note: we accept lower case because that
           is patched in pc_ino2dos() at a lower level */
        if ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') )
            return(FALSE); /* Valid */
        if (c >= 0xa1 && c <= 0xdf) /* Katakana */
            return(FALSE); /* Valid */
        if (islfn)
            return (_illegal_lfn_char(c)); /* same as ascii */
        return (_illegal_alias_char(c)); /* same as ascii */
    }
    else /* length is 2 */
    {
        p++;
        c = *p;
        if (c >= 0x40 && c <= 0x7e)
            return(FALSE); /* Valid */
        if (c >= 0x80 && c <= 0xfc)
            return(FALSE); /* Valid */
        return(TRUE);      /* Invalid */
    }
}
/* Return TRUE if illegal for an 8 dot 3 file */
BOOLEAN _illegal_jis_alias_char(byte *p)
{
    return(_illegal_jis_file_char(p, 0));
}

/*****************************************************************************
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
*****************************************************************************/

BOOLEAN pc_valid_sfn(byte *filename)    /* __fn__ */
{
    int len,period_count,ext_start;
    BOOLEAN badchar;
    byte name_part[9];

    int  char_len;
    byte *pname;
    if(name_is_reserved(filename)) return(FALSE);

    pname = filename;
    len = 0;
    badchar=FALSE;
    period_count=0;
    ext_start=0;
    name_part[0] = 0;
    while(*pname)
    {
        if(_illegal_jis_alias_char(pname)) badchar = TRUE;
        if(*pname == '.')
        {
            ext_start = len+1;
            period_count++;
        }

        char_len = jis_char_length(pname);
        if (!ext_start && len < 8)
        {
            if (char_len == 2)      /* end the test name for reserved */
                name_part[len] = 0;
            else
                name_part[len] = *pname;
            name_part[len+1] = 0;
        }
        pname += char_len;
        len += char_len;
    }
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

BOOLEAN _illegal_jis_lfn_char(byte *p)
{
    return(_illegal_jis_file_char(p, 1));
}


/***************************************************************************
    validate_filename - See if filename is a valid long file name

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
    int len;
    byte name_part[9];

    RTFS_ARGSUSED_PVOID((void *) ext);


    for(len=0; *filename; len++)
    {
        if (_illegal_jis_lfn_char(filename))
            return(FALSE);
        if (len < 5)    /* handles lpt1, aux, con etc */
        {
                if(*filename == '.')
                {
                    name_part[len] = 0;
                    if(name_is_reserved(name_part)) return(FALSE);
                }
                else
                {
                    name_part[len] = *filename;
                    if (*(filename+1) == 0)
                    {
                        name_part[len+1] = 0;
                        if(name_is_reserved(name_part)) return(FALSE);
                    }
                }
        }
        filename = jis_increment(filename);
    }

    if( (len == 0) || (len > 255) ) return(FALSE);
    return(TRUE);
}

/* JIS Version */
BOOLEAN pc_cs_malias(byte *alias, byte *input_file, int try) /*__fn__*/
{
    int n,s;
    byte filename[9],fileext[4];
    byte *p_in, *p_in_ext, *p_temp, *p_temp_2;
    int char_len, jis_ext_len;

    /* Fill filename[8] with spaces before we start. */
    rtfs_memset(filename, ' ', 8);

    /* Check if invalid short file name.Case is ignored. If ignored we fix it later !  */
    if ((try == -1) && !pc_valid_sfn((byte *)input_file))
        return(FALSE);

    /* Process the JIS alias name */
    p_in = input_file;
    while(*p_in =='.' || *p_in ==' ') p_in = jis_increment(p_in);

    /* find extension start */
    /* p_in_ext holds the position right */
    /* after the last period (or it is 0 */
    p_in_ext = 0;
    p_temp = p_in;
    while(*p_temp)
    {
        if (*p_temp =='.')
        {
            p_temp = jis_increment(p_temp);
            p_in_ext = p_temp;
        }
        else
            p_temp = jis_increment(p_temp);
    }

    p_temp_2 = &fileext[0];
    jis_ext_len = 0;        /* We'll use this later to append ext to alias */
    if(p_in_ext && *p_in_ext!=0){
        /* copy extension to fileext[] */
        p_temp = p_in_ext;
        for(s=0; *p_temp!=0 && s<3;p_temp+=char_len)
        {
            char_len = jis_char_length(p_temp);
            if(*p_temp!=' ')
            {
                /* finish if 2 bite jis overflows 3 */
                if(s==2&&char_len==2)
                {
                    break;
                }
                /* use '_' if illegal*/
                else if(_illegal_jis_alias_char(p_temp))
                {
                    *p_temp_2++ = '_';
                    s += 1;
                }
                else
                {
                    *p_temp_2++ = *p_temp;
                    if (char_len==2)
                        *p_temp_2++ = *(p_temp+1);
                    s += char_len;
                }
            }
        }
        jis_ext_len = s;    /*  Save for later. bytes in */
    }
    *p_temp_2 = 0;  /* NULL terminate extention */

    /* copy file name to filename[], filtering out spaces, periods and
        replacing characters illegal in alias names with '_' */
    p_temp   = input_file;
    p_temp_2 = filename;

    for(s=0; *p_temp!=0 && s<8; p_temp += char_len)
    {
        if (p_in_ext && p_temp>=p_in_ext)   /* hit extension ? */
            break;
        char_len = jis_char_length(p_temp);
        /* break and use ' ' if 2 bite jis overflows 6 */
        if(s==5&&char_len==2)
        {
            break;
        }
        else if(*p_temp!=' ' && *p_temp !='.')
        {
            if(_illegal_jis_alias_char(p_temp))
            {
                 *p_temp_2++ = '_';
                 s += 1;
            }
            else
            {
                *p_temp_2++ = *p_temp;
                if (char_len==2)
                    *p_temp_2++ = *(p_temp+1);
                s += char_len;
            }
        }
    }
    filename[8]=0; /* null terminate filename[] */

    pc_ascii_str2upper(filename,filename);
    pc_ascii_str2upper(fileext,fileext);

    /* append (TEXT[])i to filename[] */
    if (try != -1)
    {
        for(n=7,s=try; s>0 && n>0; s/=10,n--)
        {
            filename[n] = (byte)(((byte)s%10)+'0');
         }
         if(n==0 && s>0)
             return(FALSE);
         else
             filename[n]='~';
    }

        p_temp_2 = alias;
        p_temp = filename;

        /* copy filename[] to alias[], filtering out spaces */
        s = 0;
        while(*p_temp)
        {
            char_len = jis_char_length(p_temp);

            if (s == 7 && char_len == 2)
                break;
            if(*p_temp!=' ')
            {
                *p_temp_2++=*p_temp++;
                if (char_len == 2)
                    *p_temp_2++=*p_temp++;
                s += char_len;
                if (s == 8)
                    break;

            }
            else
                p_temp++;

        }
        if(jis_ext_len != 0)
        {
            *p_temp_2++='.'; /* insert separating period */

            /* copy fileext[] to alias[] */
            for(s=0; s < jis_ext_len; s++)
            {
                *p_temp_2++ = fileext[s];
            }
        }
    *p_temp_2=0; /* null terminate alias[] */
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
        else
        {
            if (_illegal_jis_alias_char(&name[i]))
                return(FALSE);
            /* If it is a 2 byte char advance i */
            if (jis_char_length(&name[i])==2)
            {
                if (i == last)
                    return(FALSE);  /* two byte at the end. no good */
                i++;
            }
        }
    }
    return(TRUE);
}

BOOLEAN validate_filename(byte * name, byte * ext) /*__fn__*/
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
byte save_char;
byte *save_p;
BOOLEAN ret_val;

    /* Kludge. never match a deleted file */
    if (*p == PCDELETE)
      return (FALSE);
    save_p = p;
    save_char = *p;
    if(save_char == 0x05)
        *p = 0xe5;          /* JIS KANJI char */

    ret_val = TRUE;
    while (size > 0)
    {
        if(dowildcard)
        {
            if (*pattern == '*')    /* '*' matches the rest of the name */
                goto ret;
            if (*pattern != '?' && !jis_compare_nc(pattern,p))
            {
                ret_val = FALSE;
                goto ret;
            }
        }
        else
        {
            if (!jis_compare_nc(pattern,p))
            {
                ret_val = FALSE;
                goto ret;
            }
        }
        size -= jis_char_length(p);
        p = jis_increment(p);
        pattern = jis_increment(pattern);
    }
ret:
    *save_p = save_char;
    return(ret_val);
}

BOOLEAN pc_patcmp_3(byte *p, byte *pattern, BOOLEAN dowildcard)  /* __fn__*/
{
int size = 3;
BOOLEAN ret_val;

    ret_val = TRUE;
    while (size > 0)
    {
        if(dowildcard)
        {
            if (*pattern == '*')    /* '*' matches the rest of the name */
                goto ret;
            if (*pattern != '?' && !jis_compare_nc(pattern,p))
            {
                ret_val = FALSE;
                goto ret;
            }
        }
        else
        {
            if (!jis_compare_nc(pattern,p))
            {
                ret_val = FALSE;
                goto ret;
            }
        }
        size -= jis_char_length(p);
        p = jis_increment(p);
        pattern = jis_increment(pattern);
    }
ret:
    return(ret_val);
}

#endif


/* map_jis_input() - take an ascii input string and return a string with
   containing the input but convert escaped values into hex in the output
   escape <XX to 8 bit hex, >XXXX to 16 bit hex
   Uses static storage, allows 3 buffers. More can be added
   Used only in test programs.
   MAPINPUT() calls this routine to convert input ASCII to JIS
*/

#define NJISINBUF 3

byte btoc(byte b)
{
byte c;
    if (b >= '0' && b <= '9') c = (byte)(b - '0');
    else if (b >= 'a' && b <= 'f') c = (byte)(10 + b - 'a');
    else if (b >= 'A' && b <= 'F') c = (byte)(10 + b - 'A');
    else c = 0; /* Default. bad input to zero */
    return(c);}

byte btohex(byte *pb){
    byte c;
    c = btoc(*pb);
    c <<= 4;
    pb++;
    c = (byte) (c + btoc(*pb));
    return(c);
}


byte jis_in_buf[NJISINBUF][255];
int  cur_jis_in_buf = 0;

byte *map_jis_input(byte *pin)
{
byte *pout;


    pout =  &jis_in_buf[cur_jis_in_buf][0];
    while (*pin)
    {
        if (*pin == '<') /* 8 bit hex esc */
        {
            pin++;
            *pout++ = btohex(pin);
            pin+=2;
        }
        else if (*pin == '>') /* 16 bit hex esc */
        {
            pin++;
            *pout++ = btohex(pin);
            pin+=2;
            *pout++ = btohex(pin);
            pin+=2;
        }
        else
            *pout++ = *pin++;

    }
    *pout = 0;

    /* Set up return */
    pout =  &jis_in_buf[cur_jis_in_buf][0];
    /* Increment the pointer */
    cur_jis_in_buf += 1;
    if (cur_jis_in_buf == NJISINBUF)
        cur_jis_in_buf = 0;
    return(pout);
}



#define NJISOUTBUF 3

byte btoal(byte c)
{byte a; c &= 0xf; if (c < 10) a = (byte) (c + '0'); else a = (byte) (c-10 + 'A');
    return(a);}
byte btoah(byte c)
{byte a;a = (byte)((c>>4)&0xf); return(btoal(a));}


byte jis_out_buf[NJISOUTBUF][255];
int  cur_jis_out_buf = 0;

byte *map_jis_output(byte *pin)
{
byte *pout;
int l;

    pout =  &jis_out_buf[cur_jis_out_buf][0];
    while (*pin)
    {
        if (*pin & 0x80) /* JIS Character */
        {
            l = jis_char_length(pin);
            *pout++='[';
            *pout++ = btoah(*pin);
            *pout++ = btoal(*pin++);
            if (l == 2)
            {
                *pout++ = btoah(*pin);
                *pout++ = btoal(*pin++);
            }
            *pout++=']';
        }
        else
            *pout++ = *pin++;

    }
    *pout = 0;

    /* Set up return */
    pout =  &jis_out_buf[cur_jis_out_buf][0];
    /* Increment the pointer */
    if (cur_jis_out_buf == NJISOUTBUF)
        cur_jis_out_buf = 0;
    return((byte *)pout);
}

#endif

