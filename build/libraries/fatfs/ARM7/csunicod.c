/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 2002
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* UNICODE.C - Contains UNCICODE string manipulation and character conversion routines */


#include <rtfs.h>

#if (INCLUDE_CS_UNICODE)

BOOLEAN _illegal_alias_char(byte ch);
#if (KS_LITTLE_ENDIAN)
#define BYTE_LOW  1 /* Numeric low byte of a character */
#define BYTE_HIGH 0 /* Numeric high byte of a character */
#else
#define BYTE_LOW  0 /* Numeric low byte of a character */
#define BYTE_HIGH 1 /* Numeric high byte of a character */
#endif

void pc_ascii_byte2upper(byte *to, byte *from)
{
byte c;
    c = *from;
    if  ((c >= 'a') && (c <= 'z'))
        c = (byte) ('A' + c - 'a');
    *to = c;
}

void pc_unicode_byte2upper(byte *to, byte *from)  /* __fn__*/
{
byte c;
    if (*(from+BYTE_LOW)==0)
    {
        c = *(from+BYTE_HIGH);
        if  ((c >= 'a') && (c <= 'z'))
            c = (byte) ('A' + c - 'a');
        *(to+BYTE_LOW)= 0;
        *(to+BYTE_HIGH)= c;
    }
    else
    {
        *to++ = *from++;
        *to = *from;
    }
}


int unicode_ascii_index(byte *p, byte base)
{
byte c[2];
int index;

    pc_unicode_byte2upper(c, p);
    if (c[BYTE_LOW]==0)
    {
        index = (int) (c[BYTE_HIGH] - base);
    }
    else
    {
        index = c[BYTE_HIGH];
        index <<= 8;
        index += c[BYTE_LOW];
    }
    return(index);
}

int unicode_compare(byte *p1, byte *p2)
{
    if (*p1 == *p2 && *(p1+1) == *(p2+1))
        return(1);
    else
        return(0);
}

int ascii_compare_nc(byte *p1, byte *p2)
{
byte c,d;
    if (*p1 == *p2)
        return(1);
    else
    {
        pc_ascii_byte2upper(&c, p1);
        pc_ascii_byte2upper(&d, p2);
        return(c == d);
    }
}

int unicode_compare_nc(byte *p1, byte *p2)
{
byte cp1[2];
byte cp2[2];
    pc_unicode_byte2upper(cp1,p1);
    pc_unicode_byte2upper(cp2,p2);
    if (unicode_compare(cp1, cp2))
        return(1);
    else 
        return(0);
}

byte *unicode_goto_eos(byte *p)
{
    while ((*p) || *(p+1)) p+=2;
    return(p);
}

void unicode_drno_to_letter(byte *p,int driveno)
{
    driveno += (int) 'A';
    *(p+BYTE_LOW) = 0;
    *(p+BYTE_HIGH) = (byte) driveno;
}
int unicode_cmp_to_ascii_char(byte *p, byte c)
{
    if (*(p+BYTE_LOW) == 0)
        return ( c ==*(p+BYTE_HIGH) );
    return(0);
}

void unicode_assign_ascii_char(byte *p, byte c)
{
    *(p+BYTE_LOW) = 0;
    *(p+BYTE_HIGH) = c;
}

void map_ascii_to_unicode(byte *unicode_to, byte *ascii_from)
{
byte *p;

    p = unicode_to;
    while (*ascii_from)
    {
        *(p+BYTE_LOW) = 0;
        *(p+BYTE_HIGH) = *ascii_from++;
        p+= 2;
    }
    *p++ = 0;
    *p++ = 0;
}

void map_unicode_to_ascii(byte *to, byte *from)
{
    while (*from || *(from+1))
    {
        if (*(from+BYTE_LOW) == 0)
            *to++ = *(from+BYTE_HIGH);
        else
            *to++ = '_';
        from += 2;
    }
    *to = 0;
}

void pc_ascii_strn2upper(byte *to, byte *from, int n)  /* __fn__*/
{
    int i;
    byte c;
    for (i = 0; i < n; i++)
    {
        c = *from++;
        if  ((c >= 'a') && (c <= 'z'))
            c = (byte) ('A' + c - 'a');
        *to++ = c;
    }
}

void pc_ascii_str2upper(byte *to, byte *from)  /* __fn__*/
{
        byte c;
        while(*from)
        {
            c = *from++;
            if  ((c >= 'a') && (c <= 'z'))
                c = (byte) ('A' + c - 'a');
            *to++ = c;          
        }
        *to = '\0';
}

byte * rtfs_cs_strcat(byte * targ, byte * src)    /*__fn__*/
{
byte *p;
    p = unicode_goto_eos(targ);
    rtfs_cs_strcpy(p, src);
    return targ;
}


int rtfs_cs_strcmp(byte * s1, byte * s2)
{
    word w1, w2;

    while (CS_OP_IS_NOT_EOS(s1) && CS_OP_IS_NOT_EOS(s2) && unicode_compare(s1, s2))
    {
        s1 += 2;
        s2 += 2;
    }

    w1 = (word)*(s1+BYTE_HIGH); w1 = (word) w1 << 8; w1 = (word)w1 + (word) *(s1+BYTE_LOW);
    w2 = (word)*(s2+BYTE_HIGH); w2 = (word) w2 << 8; w2 = (word)w2 + (word) *(s2+BYTE_LOW);

    if (w1 == w2)            return 0;
    else if (w1 < w2)        return -1;
    else                     return 1;
}

int rtfs_cs_strcpy(byte * targ, byte * src)
{
int loop_count = 0;
    while (CS_OP_IS_NOT_EOS(src))
    {
        *targ++ = *src++;
        *targ++ = *src++;
        loop_count++;
    }
    *targ++ = 0;
    *targ = 0;
    return (loop_count);
}

/* return number of unicode chars in a string */
int rtfs_cs_strlen(byte * string)   /*__fn__*/
{
int len;
    len = 0;
    while (CS_OP_IS_NOT_EOS(string))
    {
        len += 1;
        string += 2;
    }
    return (len);
}

BOOLEAN validate_filename(byte * name, byte * ext)
{
    int len;
    byte *pa, *pu, uni_buffer[16], ascii_buffer[8];

    RTFS_ARGSUSED_PVOID((void *) ext);

    /* Check short filenames to see if they are reserved names, The test 
       is done in ascii because we use this for aliases */
    len = 0;
    pu = uni_buffer; pa = name;
    /* Make a unicode string up to DOT or Zero */
    while (CS_OP_IS_NOT_EOS(pa))
    {
        if (CS_OP_CMP_ASCII(pa,'.'))
            break;
        CS_OP_CP_CHR(pu, pa);
        CS_OP_INC_PTR(pu);
        CS_OP_INC_PTR(pa);
        CS_OP_TERM_STRING(pu);
        len += 1;
        if (len > 4)
            break;
    }
    if (len && len <= 4)
    {
        map_unicode_to_ascii(ascii_buffer, uni_buffer);
        if(name_is_reserved(ascii_buffer)) return(FALSE);
    }

    if (*(name+BYTE_LOW)==0)
        if (*(name+BYTE_HIGH) == ' ')
           return(FALSE);
    len = 1;
    name += 2;
    while (CS_OP_IS_NOT_EOS(name))
    {
        if (*(name+BYTE_LOW)==0)
            if (_illegal_lfn_char(*(name+BYTE_HIGH)))
                return(FALSE);
        name += 2;
        len += 1;
    }
    if(len > 255) return(FALSE);
    return(TRUE);
}

BOOLEAN pc_ascii_malias(byte *alias, byte *input_file, int try);

/* UNICODE Version */
BOOLEAN pc_cs_malias(byte *alias, byte *input_file, int try) /*__fn__*/
{
    BLKBUFF *scratch;
    BLKBUFF *scratch1;
    byte *ascii_input_file;
    byte *ascii_alias;
    BOOLEAN ret_val = FALSE; 

    /* We know UNICODE file names aren't legal aliases */
    if(try == -1)
        return(FALSE); 

    scratch = scratch1 = 0;

    scratch = pc_scratch_blk();
    scratch1 = pc_scratch_blk();
    if (!scratch || !scratch1)
        goto ex_it;
    ascii_input_file = scratch->data;
    ascii_alias = scratch1->data;

    /* Map the unicode to ascii and create an ascii alias */
    map_unicode_to_ascii(ascii_input_file, input_file);
    if (!pc_ascii_malias(ascii_alias, ascii_input_file, try))
        goto ex_it;
    /* Now back to unicode */
    map_ascii_to_unicode(alias, ascii_alias);
    ret_val = TRUE;
ex_it:
    if (scratch)
        pc_free_scratch_blk(scratch);
    if (scratch1)
        pc_free_scratch_blk(scratch1);
    return(ret_val);
}


BOOLEAN pc_patcmp_8(byte *p, byte *pattern, BOOLEAN dowildcard)  /* __fn__*/
{
byte ascii_pattern[9];
int size = 8;

    map_unicode_to_ascii(ascii_pattern, pattern);
    pattern = ascii_pattern;
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
byte ascii_pattern[9];
int size = 3;

    map_unicode_to_ascii(ascii_pattern, pattern);
    pattern = ascii_pattern;

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

void lfn_chr_to_unicode(byte *to, byte *fr)
{
#if (KS_LITTLE_ENDIAN)
    *to = *fr++;
    *(to+1)   = *fr;
#else /* make sure UNICODE str is in Intel byte order on disk */
    *(to+1)   = *fr++;
    *to = *fr;
#endif                 
}
void unicode_chr_to_lfn(byte *to, byte *fr)
{
#if (KS_LITTLE_ENDIAN)
    *to = *fr++;
    *(to+1)   = *fr;
#else /* make sure UNICODE str is in Intel byte order on disk */
    *(to+1)   = *fr++;
    *to = *fr;
#endif                 
}

byte *pc_ascii_mfile(byte *to, byte *filename, byte *ext);


/* Version of MFILE that converts path from byte orientation to 
   native char set before returning. */
byte *pc_cs_mfile(byte *to, byte *filename, byte *ext)
{
byte temp_to[13];
    pc_ascii_mfile(temp_to, filename, ext);
    map_ascii_to_unicode(to, temp_to);
    return(to);
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
        PC_MFILE  - Build a file spec (xxx.yyy) from a file name and extension

 Description
        Fill in to with a concatenation of file and ext. File and ext are
        not assumed to be null terminated but must be blank filled to [8,3]
        chars respectively. 'to' will be a null terminated string file.ext.

        ASCII character function only. Unicode not required

 Returns
        A pointer to 'to'.
This is byte oriented. See pc_cs_mfile() for unicode oriented.
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
BOOLEAN pc_valid_ascii_sfn(byte *filename);

BOOLEAN pc_ascii_malias(byte *alias, byte *input_file, int try)
{
    int n,i,s;
    byte filename[9],fileext[4];

    /* Fill filename[8] with spaces before we start. */
    rtfs_memset(filename, ' ', 8);

    /* See if no change necessary! */
    if(try == -1)
        return(pc_valid_ascii_sfn((byte *)input_file));
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

     /* append (TEXT[])i to filename[] */
     for(n=7,s=try; s>0 && n>0; s/=10,n--) 
     {
         filename[n] = (byte)(((byte)s%10)+'0');
     }
     if(n==0 && s>0)
         return(FALSE);
     else
         filename[n]='~'; 

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

BOOLEAN pc_valid_ascii_sfn(byte *filename)
{
    int len,period_count,ext_start;
    BOOLEAN badchar;
    if(name_is_reserved(filename)) return(FALSE);
    for(len=0,badchar=FALSE,period_count=0,ext_start=0; filename[len]!=0; len++)
    {
        if(_illegal_alias_char(filename[len])) badchar = TRUE;
        if(filename[len] == '.') 
        {
            ext_start = len+1;
            period_count++;
        }
    }

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

byte print_buffer[132];

/* Take a string the may be either ASCI unicode '\0''C' or just asci 'C' 
   and return an asci printable version of the string */
byte *unicode_make_printable(byte *p)
{
int i;
byte c;
int is_unicode;

    is_unicode = 0;

    if (*(p+BYTE_LOW)!=0) /* Most likely it is ascii */
    {
        rtfs_strcpy(print_buffer, p);
        return(&print_buffer[0]);

    }

    i = 0;
    for (;;)
    {
        if (*(p+BYTE_LOW)==0)
        {
            c = *(p+BYTE_HIGH);
            if (c == 0)
                break;
            else
                print_buffer[i++] = c;
        }
        p += 2;
        
    }
    print_buffer[i] = 0;
    return (&print_buffer[0]);
}
#endif

