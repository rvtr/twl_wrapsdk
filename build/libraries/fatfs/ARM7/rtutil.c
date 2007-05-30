/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTUTIL.C - String manipulation and byte order conversion routines */


#include <rtfs.h>

/******************************************************************************
PC_PARSEDRIVE -  Get a drive number from a path specifier

  Description
  Take a path specifier in path and extract the drive number from it.
  If the second character in path is ':' then the first char is assumed
  to be a drive specifier and 'A' is subtracted from it to give the
  drive number. If the drive number is valid, driveno is updated and 
  a pointer to the text just beyond ':' is returned. Otherwise null
  is returned.
  If the second character in path is not ':' then the default drive number
  is put in driveno and path is returned.
  
    Unicode character function. Unicode is required
    
      Returns
      Returns NULL on a bad drive number otherwise a pointer to the first
      character in the rest of the path specifier.
      ***************************************************************************/
      
      /* Get the drive number form the path. Make sure one is provided */
      /* and that it is valid. */
      /* returns -1 if not valid otherwise return the driveno */
      
      /* UNICODE/ASCI/JIS Version */
int pc_path_to_driveno(byte  *path)                           /* __fn__*/
{
    byte *p;
    int drivenumber;
    p = path;
    if (CS_OP_IS_EOS(p))
        return(-2);
    CS_OP_INC_PTR(p);
    if (CS_OP_CMP_ASCII(p,':'))
    {
        drivenumber = CS_OP_ASCII_INDEX(path,'A');
        if (drivenumber < 0 || drivenumber > 25)
        {
            rtfs_set_errno(PEINVALIDDRIVEID); /* pc_path_to_driveno: invalid drive number */
            drivenumber = -1;
        }
    }
    else
        drivenumber = -2;
    return(drivenumber);
}

/* Parse drive part from a string - return -2 if no 'A:' -1 if A: 
and not valid drive id */
int pc_parse_raw_drive(byte  *path)                             /* __fn__*/
{
    int dno;
    dno = pc_path_to_driveno(path);
    if (dno < 0 || !pc_validate_driveno(dno))
        return (-1);
    else
        return(dno);
}

/* Extract drive no from D: or use defualt. return the rest of the string
or NULL if a bad drive no is requested */
byte *pc_parsedrive(int *driveno, byte  *path)                              /* __fn__*/
{
    byte *p = path;
    int dno;
    
    /* get drive no */
    dno = pc_path_to_driveno(path);
    if (dno == -2)  /* D: Not specified. Use current default  */
    {
        dno = pc_getdfltdrvno();
        /* Make sure default drive number is valid */
        if (!pc_validate_driveno(dno))
            return(0);
    }
    else if (dno == -1) /* D: specified but bogus */
        return(0);
    else  /* D: Specified. validate */
    {
        if (!pc_validate_driveno(dno))
            return(0);
        /* SKIP D and : */
        CS_OP_INC_PTR(p);
        CS_OP_INC_PTR(p);
    }
    
    *driveno = dno;
    return (p);
}

/***************************************************************************
        PC_MPATH  - Build a path sppec from a filename and pathname

 Description
        Fill in to with a concatenation of path and filename. If path 
        does not end with a path separator, one will be placed between
        path and filename.

        TO will be null terminated.

 Returns
        A pointer to 'to'.

*****************************************************************************/
byte *pc_mpath(byte *to, byte *path, byte *filename)                                /* __fn__*/
{
        byte *retval = to;
        byte *p;
        byte c[2];

        c[0] = c[1] = 0;
        p = path;
        while (CS_OP_IS_NOT_EOS(p))
        {
            if (CS_OP_CMP_ASCII(p,' '))
                break;
            else
            {
                CS_OP_CP_CHR(c, p);
                CS_OP_CP_CHR(to, p);
                CS_OP_INC_PTR(p);
                CS_OP_INC_PTR(to);
            }
        }
        /* Put \\ on the end f not there already, but not if path was null */       
        if (p != path && !CS_OP_CMP_ASCII(c,'\\'))
        {
            CS_OP_ASSIGN_ASCII(to,'\\');
            CS_OP_INC_PTR(to);
        }

        p = filename;
        while (CS_OP_IS_NOT_EOS(p))
        {
            CS_OP_CP_CHR(to, p);
            CS_OP_INC_PTR(p);
            CS_OP_INC_PTR(to);
        }
        CS_OP_TERM_STRING(to);
        return (retval);
}

/* Byte oriented - search comma separated list in set for filename */
BOOLEAN pc_search_csl(byte *set, byte *string)
{
byte *p;
    if (!set || !string)
        return(FALSE);
    while(*set)
    {
        p = string;
        while (*set && *p && (*set == *p))
        {
            set++;
            p++;
        }
        if ((*set == 0 || *set == ',') && *p == 0)
            return(TRUE);
        while (*set && *set != ',') set++;
        while (*set == ',') set++;
        if (!(*set))
            break;
    }
    return(FALSE);
}

BOOLEAN name_is_reserved(byte *filename)    /* __fn__*/
{
    return (
    pc_search_csl(rtfs_strtab_user_string(USTRING_SYS_UCRESERVED_NAMES), filename) ||
    pc_search_csl(rtfs_strtab_user_string(USTRING_SYS_LCRESERVED_NAMES), filename) 
    );
}

/* ******************************************************************** */
/* KEEP COMPILER HAPPY ROUTINES */
/* ******************************************************************** */
/* Used to keep the compiler happy  */
void RTFS_ARGSUSED_PVOID(void * p)  /*__fn__*/
{
    p = p;  
}

/* Used to keep the compiler happy  */
void RTFS_ARGSUSED_INT(int i)       /*__fn__*/
{
    i = i;
}

/*****************************************************************************
PC_CNVRT -  Convert intel byte order to native byte order.

  Summary
  #include <rtfs.h>
  
    dword to_DWORD (from)  Convert intel style 32 bit to native 32 bit
    byte *from;
    
      word to_WORD (from)  Convert intel style 16 bit to native 16 bit
      byte *from;
      
        void fr_WORD (to,from) Convert native 16 bit to 16 bit intel
        byte *to;
        word from;
        
          void fr_DWORD (to,from) Convert native 32 bit to 32 bit intel
          byte *to;
          dword from;
          
            Description
            This code is known to work on 68K and 808x machines. It has been left
            as generic as possible. You may wish to hardwire it for your CPU/Code
            generator to shave off a few bytes and microseconds, be careful though
            the addresses are not guaranteed to be word aligned in fact to_WORD AND
            fr_WORD's arguments are definately NOT word alligned when working on odd
            number indeces in 12 bit fats. (see pc_faxx and pc_pfaxx().
            
              Note: Optimize at your own peril, and after everything else is debugged.
              
                Bit shift operators are used to convert intel ordered storage
                to native. The host byte ordering should not matter.
                
                  Returns
                  
                    Example:
                    See other sources.
                    
                      *****************************************************************************
*/

/* Convert a 32 bit intel item to a portable 32 bit */
dword to_DWORD ( byte *from)                                                                        /*__fn__*/
{
    dword res;
#if (KS_LITTLE_ENDIAN && KS_LITTLE_ODD_PTR_OK)
    res = ((dword) *((dword *)from));
#else
    dword t;
    t = ((dword) *(from + 3)) & 0xff;
    res = (t << 24);
    t = ((dword) *(from + 2)) & 0xff;
    res |= (t << 16);
    t = ((dword) *(from + 1)) & 0xff;
    res |= (t << 8);
    t = ((dword) *from) & 0xff;
    res |= t;
#endif
    return(res);
}

/* Convert a 16 bit intel item to a portable 16 bit */
word to_WORD ( byte *from)                                                                      /*__fn__*/
{
    word nres;
#if (KS_LITTLE_ENDIAN && KS_LITTLE_ODD_PTR_OK)
    nres = ((word) *((word *)from));
#else
    word t;
    t = (word) (((word) *(from + 1)) & 0xff);
    nres = (word) (t << 8);
    t = (word) (((word) *from) & 0xff);
    nres |= t;
#endif
    return(nres);
}

/* Convert a portable 16 bit to a  16 bit intel item */ 
void fr_WORD ( byte *to,  word from)                                            /*__fn__*/
{
#if (KS_LITTLE_ENDIAN && KS_LITTLE_ODD_PTR_OK)
    *((word *)to) = from;
#else
    *to             =       (byte) (from & 0x00ff);
    *(to + 1)   =   (byte) ((from >> 8) & 0x00ff);
#endif
}

/* Convert a portable 32 bit to a  32 bit intel item */
void fr_DWORD ( byte *to,  dword from)                                          /*__fn__*/
{
#if (KS_LITTLE_ENDIAN && KS_LITTLE_ODD_PTR_OK)
    *((dword *)to) = from;
#else
    *to = (byte) (from & 0xff);
    *(to + 1)   =  (byte) ((from >> 8) & 0xff);
    *(to + 2)   =  (byte) ((from >> 16) & 0xff);
    *(to + 3)   =  (byte) ((from >> 24) & 0xff);
#endif
}


