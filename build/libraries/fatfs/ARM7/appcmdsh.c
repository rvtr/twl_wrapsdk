/**************************************************************************
    APPTSTSH.C   - Command driven test shell for embedded file manager.

Summary
    TSTSH

 Description
    Interactive shell program designed to allow testing of the file manager.
    Commands are:

    BRA V1 < = ! < > > V2 LABEL
    CAT PATH
    CD PATH or CD to display PWD
    CHSIZE FILENAME newsize
    CLOSE FDNO
    COPY PATH PATH
    DELETE PATH
    DELTREE PATH
    DIFF PATH PATH
    DIR PATH
    DSKOPEN:  D:
    DSKSEL    D:
    ECHO: [args]
    FILLFILE PATH PATTERN NTIMES
    FORMAT (routine will prompt for arguments)
    HELP:
    LOAD [FILENAME]
    LSTOPEN (lists all open file decscriptors)
    MKDIR PATH
    MKFS E:
    OP V1 <+,*,-,/,%> V2 DEST <A-Z>
    QUIET ON,OFF
    QUIT
    READ FDNO
    RENAME PATH NEWNAME
    STAT   PATH
    FSTAT  PATH
    RMDIR PATH
    RNDOP PATH RECLEN
    RUN
    SEEK FDNO RECORD
    SET <A-Z> VALUE
    SETATTR D:PATH RDONLY|HIDDEN|SYSTEM|ARCHIVE|NORMAL
    WRITE FDNO QUOTED DATA

Returns

Example:
*****************************************************************************
*/

#include <rtfsapi.h>
#include <portconf.h>   /* For included devices */

#define INCLUDE_STRESS_TESTS   0 /* Echo command fires off stress test */

do_display_errno(void)
{
#if (INCLUDE_CS_UNICODE)
    RTFS_PRINT_STRING_2(USTRING_SYS_NULL,(byte *)L"Errno == ",0);
#else
    RTFS_PRINT_STRING_2(USTRING_SYS_NULL,(byte *)"Errno == ",0);
#endif
    RTFS_PRINT_LONG_1((dword)get_errno(), PRFLG_NL);
}
#define DISPLAY_ERRNO(ROUTINE) do_display_errno();


void rtfs_print_format_dir(byte *display_buffer, DSTAT *statobj);
void rtfs_print_format_stat(byte *display_buffer, ERTFS_STAT *st);

#if (INCLUDE_HOSTDISK)
int domkhostdisk(int agc, byte **agv);
int domkrom(int agc, byte **agv);
#endif

/* 10-24-2000 added LBA formatting. Submit to main tree */

/* Set to one to compile in check disk.  */
#define INCLUDE_CHKDSK 1
#if (INCLUDE_PCMCIA)
void  KS_FAR mgmt_isr(void);
#endif

byte shell_buf[1024];
byte working_buf[100];      /* used by lex: must be global */


typedef struct prg_line
{
    struct prg_line *pnext_line;
    int  label;
    byte command[1];
} PRGLINE;

typedef struct dispatcher {
    int cmd;
    int  (*proc)( int argc, byte **argv);
    int helpstr;
} DISPATCHER;

typedef struct rndfile {
    PCFD fd;
    int reclen;
    byte name[90];
    byte buff[512];
} RNDFILE;

#define MAXRND 10

RNDFILE rnds[MAXRND];

DISPATCHER *lex(int *agc, byte **agv);
byte *gnext(byte *p);
int echo( int agc, byte **agv);
int doquit(int agc, byte **agv);
int dopcmcia(int agc, byte **agv);
RNDFILE *fndrnd( PCFD fd);
int doeject(int agc, byte **agv);
int dohelp(int agc, byte **agv);
int dodskop(int agc, byte **agv);
int dodskcl(int agc, byte **agv);
int doselect(int agc, byte **agv);
int rndop(int agc, byte **agv);
int doclose(int agc, byte **agv);
int doseek(int agc, byte **agv);
int doread(int agc, byte **agv);
int dowrite(int agc, byte **agv);
int dolstop(int agc, byte **agv);
int domkdir(int agc, byte **agv);
int dormdir(int agc, byte **agv);
int dorm(int agc, byte **agv);
int dodeltree(int agc, byte **agv);
int domv(int agc, byte **agv);
int docwd(int agc, byte **agv);
int dols(int agc, byte **agv);
int pc_seedir(byte *path);
int docat(int agc, byte **agv);
int dochsize(int agc, byte **agv);
#if (INCLUDE_CHKDSK)
int dochkdsk(int agc, byte **agv);
#endif

int docopy(int agc, byte **agv);
int dodevinfo(int agc, byte **agv);
int dodiff(int agc, byte **agv);
int domkfs(int agc, byte **agv);
int dofillfile(int agc, byte **agv);
int dostat(int agc, byte **agv);
int dogetattr(int agc, byte **agv);
int dosetattr(int agc, byte **agv);
int doformat(int agc, byte **agv);
int doregress(int agc, byte **agv);

#if (INCLUDE_CDROM)
#include <cdfs.h>
int cddodskop(int agc, byte **agv);
int cddodskcl(int agc, byte **agv);
int cddoselect(int agc, byte **agv);
int cddocwd(int agc, byte **agv);
int cddols(int agc, byte **agv);
int cddocat(int agc, byte **agv);
/*int cddocopy(int agc, byte **agv); */
/*int cddoread(int agc, byte **agv); */
#endif
#if (INCLUDE_FAILSAFE_CODE)
int dofsinit(int agc, byte **agv);
int dofscommit(int agc, byte **agv);
int dofsrestore(int agc, byte **agv);
int dofsstatus(int agc, byte **agv);
int dofstest(int agc, byte **agv);
#endif
/* From rtfsinit.c */
void print_device_names(void);


DISPATCHER cmds[] =
    {
    { USTRING_TSTSHCMD_26, dohelp,  USTRING_TSTSHHELP_26 }, /* "HELP","HELP:" */
    { USTRING_TSTSHCMD_01, docat, USTRING_TSTSHHELP_01 },   /* "CAT","CAT PATH" */
    { USTRING_TSTSHCMD_02, dochsize,  USTRING_TSTSHHELP_02},    /* "CHSIZE","CHSIZE FILENAME NEWSIZE" */
    { USTRING_TSTSHCMD_03, docwd,  USTRING_TSTSHHELP_03 },  /* "CD","CD PATH or CD to display PWD " */
#if (INCLUDE_CDROM)
    { USTRING_TSTSHCMD_04, cddocat, USTRING_TSTSHHELP_04 }, /* "CDCAT","CDCAT PATH" */
/*    { USTRING_TSTSHCMD_05, cddocopy, USTRING_TSTSHHELP_05 },   "CDCOPY","CDCOPY PATH PATH"  */
/*    { USTRING_TSTSHCMD_06, cddoread, USTRING_TSTSHHELP_06 },   "CDREAD","CDREAD PATH" */
    { USTRING_TSTSHCMD_07, cddocwd,  USTRING_TSTSHHELP_07 },    /* "CDCD","CDCD PATH " */
    { USTRING_TSTSHCMD_08, cddols,  USTRING_TSTSHHELP_08 }, /* "CDDIR","CDDIR PATH" */
    { USTRING_TSTSHCMD_09, cddodskcl,  USTRING_TSTSHHELP_09 },  /* "CDDSKCLOSE","CDDSKCLOSE: D:" */
    { USTRING_TSTSHCMD_10, cddodskop,  USTRING_TSTSHHELP_10 },  /* "CDDSKOPEN","CDDSKOPEN: D:" */
    { USTRING_TSTSHCMD_11, cddoselect,  USTRING_TSTSHHELP_11 }, /* "CDDSKSEL","CDDSKSEL D:" */
#endif
#if (INCLUDE_CHKDSK)
    { USTRING_TSTSHCMD_12, dochkdsk,  USTRING_TSTSHHELP_12},    /* "CHKDSK","CHKDSK D: <0,1> 1 is write lost chains" */
#endif
    { USTRING_TSTSHCMD_13, doclose,  USTRING_TSTSHHELP_13 },    /* "CLOSE","CLOSE FDNO" */
    { USTRING_TSTSHCMD_14, docopy, USTRING_TSTSHHELP_14 },  /* "COPY","COPY PATH PATH" */
    { USTRING_TSTSHCMD_15,dorm,  USTRING_TSTSHHELP_15 },    /* "DELETE","DELETE PATH" */
    { USTRING_TSTSHCMD_16, dodeltree,  USTRING_TSTSHHELP_16},   /* "DELTREE","DELTREE PATH" */
    { USTRING_TSTSHCMD_17, dodevinfo,  USTRING_TSTSHHELP_17},   /* "DEVINFO","DEVINFO (Display Device Information)" */
    { USTRING_TSTSHCMD_18, dodiff, USTRING_TSTSHHELP_18 },  /* "DIFF","DIFF PATH PATH" */
    { USTRING_TSTSHCMD_19, dols,  USTRING_TSTSHHELP_19 },   /* "DIR","DIR PATH" */
    { USTRING_TSTSHCMD_20, doselect,  USTRING_TSTSHHELP_20 },   /* "DSKSEL","DSKSEL D:" */
    { USTRING_TSTSHCMD_21, echo,  USTRING_TSTSHHELP_21 },   /* "ECHO","ECHO: [args]" */
    { USTRING_TSTSHCMD_22, doeject,  USTRING_TSTSHHELP_22 },    /* "EJECT","EJECT (ejects LS-120)" */
    { USTRING_TSTSHCMD_23, dofillfile, USTRING_TSTSHHELP_23},   /* "FILLFILE","FILLFILE PATH PATTERN NTIMES" */
    { USTRING_TSTSHCMD_24, doformat, USTRING_TSTSHHELP_24}, /* "FORMAT","FORMAT (routine will prompt for arguments)" */
    { USTRING_TSTSHCMD_25, dogetattr, USTRING_TSTSHHELP_25},    /* "GETATTR","GETATTR FILE" */
    { USTRING_TSTSHCMD_27, dolstop,  USTRING_TSTSHHELP_27  },   /* "LSTOPEN","LSTOPEN (lists all open file decscriptors) " */
    { USTRING_TSTSHCMD_28, domkdir,  USTRING_TSTSHHELP_28 },    /* "MKDIR","MKDIR PATH" */
    { USTRING_TSTSHCMD_29,  dopcmcia,  USTRING_TSTSHHELP_29 },  /* "PCMCIAINT","PCMCIAINT (Force a PCMCIA mgmt Interrupt)" */
    { USTRING_TSTSHCMD_30, 0,  USTRING_TSTSHHELP_30 },  /* The Zero value will force a return */    /* "QUIT","QUIT" */
    { USTRING_TSTSHCMD_31,  doread,  USTRING_TSTSHHELP_31 },    /* "READ","READ FDNO" */
    { USTRING_TSTSHCMD_32,domv,  USTRING_TSTSHHELP_32 },    /* "RENAME","RENAME PATH NEWNAME" */
    { USTRING_TSTSHCMD_33, dormdir,  USTRING_TSTSHHELP_33 },    /* "RMDIR","RMDIR PATH" */
    { USTRING_TSTSHCMD_34, rndop,  USTRING_TSTSHHELP_34 },  /* "RNDOP","RNDOP PATH RECLEN" */
    { USTRING_TSTSHCMD_35,  doseek,  USTRING_TSTSHHELP_35 },    /* "SEEK","SEEK FDNO RECORD" */
    { USTRING_TSTSHCMD_36, dosetattr,  USTRING_TSTSHHELP_36},   /* "SETATTR","SETATTR D:PATH RDONLY|HIDDEN|SYSTEM|ARCHIVE|NORMAL" */
    { USTRING_TSTSHCMD_37, dostat,  USTRING_TSTSHHELP_37 }, /* "STAT","STAT PATH" */
    { USTRING_TSTSHCMD_38, dowrite,  USTRING_TSTSHHELP_38 },    /* "WRITE","WRITE FDNO QUOTED DATA" */
    { USTRING_TSTSHCMD_39, doregress,  USTRING_TSTSHHELP_39 },  /* "REGRESSTEST","REGRESSTEST D:" */
#if (INCLUDE_HOSTDISK)
    { USTRING_TSTSHCMD_40, domkhostdisk,  USTRING_TSTSHHELP_40 },   /* "MKHOSTDISK","MKHOSTDISK win9xpath" */
    { USTRING_TSTSHCMD_41, domkrom,  USTRING_TSTSHHELP_41 },    /* "MKROM","MKROM" */
#endif
#if (INCLUDE_FAILSAFE_CODE)
    { USTRING_TSTSHCMD_42, dofsinit,    USTRING_TSTSHHELP_42 },   /* "FAILSAFEINIT","FAILSAFEINIT" */
    { USTRING_TSTSHCMD_43, dofscommit,  USTRING_TSTSHHELP_43 },    /* "FAILSAFECOMMIT","FAILSAFECOMMIT" */
    { USTRING_TSTSHCMD_44, dofsrestore,  USTRING_TSTSHHELP_44 },    /* "FAILSAFERESTORE","FAILSAFERESTORE" */
    { USTRING_TSTSHCMD_45, dofsstatus,      USTRING_TSTSHHELP_45 },    /* "BUFFERSTAT","BUFFERSTAT D:" */
    { USTRING_TSTSHCMD_46, dofstest,      USTRING_TSTSHHELP_46 },    /* "FAILSAFETEST","FAILSAFETEST D: (must be host disk)" */
#endif
    { 0 }
    };


byte *gnext(byte *p);
void exit(int);


BOOLEAN tstsh_is_(byte *p, byte c)
{
int  index;
    index = CS_OP_ASCII_INDEX(p, 'A');
    if (index == (int) (c - CS_OP_ASCII('A')))
        return(TRUE);
    else
        return(FALSE);
}
BOOLEAN tstsh_is_yes(byte *p)
{
    return(tstsh_is_(p, CS_OP_ASCII('Y')));
}
BOOLEAN tstsh_is_no(byte *p)
{
    return(tstsh_is_(p, CS_OP_ASCII('N')));
}

long rtfs_atol(byte * s);

int rtfs_atoi(byte * s)
{
    return((int)rtfs_atol(s));
}

long rtfs_atol(byte * s)
{
long n;
BOOLEAN neg;
int index;
    /* skip over tabs and spaces */
    while ( CS_OP_CMP_ASCII(s,' ') || CS_OP_CMP_ASCII(s,'\t') )
        CS_OP_INC_PTR(s);
    n = 0;
    neg = FALSE;
    if (CS_OP_CMP_ASCII(s,'-'))
    {
        neg = TRUE;
        CS_OP_INC_PTR(s);
    }
    while (CS_OP_IS_NOT_EOS(s))
    {
        index = CS_OP_ASCII_INDEX(s, '0');

        if (index >= 0 && index <= 9)
        {
            n = n * 10;
            n += (long) index;
            CS_OP_INC_PTR(s);
        }
        else
            break;
    }

    if (neg)
        n = 0 - n;

    return(n);
}



/* ******************************************************************** */
/* THIS IS THE MAIN PROGRAM FOR THE TEST SHELL */
/* ******************************************************************** */
void  tst_shell(void)                             /* __fn__ */
{
    DISPATCHER *pcmd;
    int  agc = 0;
    byte *agv[20];
    int i;

    rtfs_memset((byte *)rnds, 0, sizeof(RNDFILE)*MAXRND);

    for (i = 0 ; i < MAXRND; i++)
        rnds[i].fd = -1;
    rtfs_print_prompt_user(UPROMPT_TSTSH2, working_buf);
    dohelp(agc, agv);

    pcmd = lex( &agc, &agv[0]);
    while (pcmd)
    {
        if (!pcmd->proc)
            return;
        i = pcmd->proc(agc, &agv[0]);
        pcmd = lex( &agc, &agv[0]);
   }
}

/* ******************************************************************** */
/* get next command; process history log */
DISPATCHER *lex(int *agc, byte **agv)      /*__fn__*/
{
    byte *cmd,*p,*p2;
    DISPATCHER *pcmds = &cmds[0];

    *agc = 0;
    /* "CMD>" */
    rtfs_print_prompt_user(UPROMPT_TSTSH1, working_buf);

    p = cmd = &working_buf[0];

    p = gnext(p);

    while (p)
    {
       if (CS_OP_CMP_ASCII(p,'"'))
       {
            p2 = p;
            CS_OP_TERM_STRING(p);
            CS_OP_INC_PTR(p2);
            *agv++ = p2;
       }
       else
           *agv++ = p;
       *agc += 1;
       p = gnext(p);
    }

    while (pcmds->cmd)
    {
        if (rtfs_cs_strcmp(cmd,rtfs_strtab_user_string(pcmds->cmd)) == 0)
            return (pcmds);
        pcmds++;
    }

    /* No match return ??? */

    return (&cmds[0]);
 }

/* Null term the current token and return a pointer to the next */
byte *gnext(byte *p)                                            /*__fn__*/
{
    byte termat;
    /* Look for space or the end of a quoted string */
    if (CS_OP_CMP_ASCII(p,'"'))
        termat = CS_OP_ASCII('"');
    else
        termat = CS_OP_ASCII(' ');

    CS_OP_INC_PTR(p);
    while (CS_OP_IS_NOT_EOS(p))
    {
        if (CS_OP_CMP_ASCII(p,termat))
        {
            CS_OP_TERM_STRING(p);    /* Null it and look at next */
            CS_OP_INC_PTR(p);
            break;
        }
        CS_OP_INC_PTR(p);
    }

    if (CS_OP_IS_EOS(p))                /* All done */
        p = 0;

    return (p);
}


int echo( int agc, byte **agv)                                  /*__fn__*/
{
    int i;
    for (i = 0; i < agc; i++)
        RTFS_PRINT_STRING_2(USTRING_SYS_NULL, *agv++, PRFLG_NL);
    return (1);
}


int dopcmcia( int agc, byte **agv)                                /*__fn__*/
{
    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *)agv);
#if (INCLUDE_PCMCIA)
    mgmt_isr();
#endif
    /* Release all resources used by the disk the disk */
    return (1);
}


/* Given a fd. return a RNDFILE record with matching fd */
RNDFILE *fndrnd( PCFD fd)                                       /*__fn__*/
{
int i;

    for (i = 0 ; i < MAXRND; i++)
    {
        if (fd == rnds[i].fd)
            return (&rnds[i]);
    }
    return (0);
}

int dohelp(int agc, byte **agv)                             /*__fn__*/
{
DISPATCHER *pcmds = &cmds[0];
byte buf[10];
int nprinted = 0;

    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *)agv);


    while (pcmds->cmd)
    {
        RTFS_PRINT_STRING_1(pcmds->helpstr, PRFLG_NL);
        if (nprinted++ > 20)
        {
            /* "Press return" */
            rtfs_print_prompt_user(UPROMPT_TSTSH2, buf);
            nprinted = 0;
        }
        pcmds++;
    }
    return (1);
}

/* EJECT D: */
BOOLEAN ide_eject_media(int driveno);

int doeject(int agc, byte **agv)                                   /*__fn__*/
{
    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *)agv);
      RTFS_PRINT_STRING_1(USTRING_TSTSH_02,PRFLG_NL); /* "Not implemented. See ide ioctl... " */
    return(0);
}



/* DSKSEL PATH */
int doselect(int agc, byte **agv)                                   /*__fn__*/
{
    if (agc == 1)
    {
        if (!pc_set_default_drive(*agv))
        {
            DISPLAY_ERRNO("pc_set_default_drive")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_03,PRFLG_NL); /* "Set Default Drive Failed" */
            return(0);
        }
        return(1);
    }
    else
    {
         RTFS_PRINT_STRING_1(USTRING_TSTSH_04, PRFLG_NL); /* "Usage: DSKSELECT D: " */
         return(0);
    }
}


/* RNDOP PATH RECLEN */
int rndop(int agc, byte **agv)                                  /*__fn__*/
{
    RNDFILE *rndf;

    if (agc == 2)
    {
        rndf = fndrnd( -1);
        if (!rndf)
            {RTFS_PRINT_STRING_1(USTRING_TSTSH_05, PRFLG_NL);} /* "No more random file slots " */
        else
        {
            rtfs_cs_strcpy(rndf->name, *agv);
            agv++;
            rndf->reclen  = (int)rtfs_atoi(*agv);
            if ((rndf->fd = po_open(rndf->name, (PO_BINARY|PO_RDWR|PO_CREAT),
                           (PS_IWRITE | PS_IREAD) ) ) < 0)
            {
                DISPLAY_ERRNO("pc_open(PO_BINARY|PO_RDWR|PO_CREAT)")
                 RTFS_PRINT_STRING_2(USTRING_TSTSH_06,rndf->name,PRFLG_NL); /* "Cant open :" */
                 /* Note: rndf->fd is still -1 on error */
            }
            else
                return (1);
       }
  }
  RTFS_PRINT_STRING_1(USTRING_TSTSH_07,PRFLG_NL); /* "Usage: RNDOP D:PATH RECLEN" */
  return (0);
}


/* CLOSE fd */
int doclose(int agc, byte **agv)                                /*__fn__*/
{
    PCFD fd;
    RNDFILE *rndf;

    if (agc == 1)
    {
        fd = rtfs_atoi(*agv);
        rndf = fndrnd( fd );
        if (!rndf)
            {RTFS_PRINT_STRING_1(USTRING_TSTSH_08,PRFLG_NL);} /* "Cant find file" */
        else
        {
            if (po_close(fd) < 0)
            {
                DISPLAY_ERRNO("po_close")
                RTFS_PRINT_STRING_1(USTRING_TSTSH_09,PRFLG_NL); /* "Close failed" */
            }
            else
             {
                rndf->fd = -1;
                return (1);
             }
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_10,PRFLG_NL); /* "Usage: CLOSE fd" */
    return (0);
}

/* SEEK fd recordno */
int doseek(int agc, byte **agv)                                 /*__fn__*/
{
    PCFD fd;
    RNDFILE *rndf;
    int recno;
    long foff;
    long seekret;

    if (agc == 2)
    {
        fd = rtfs_atoi(*agv);
        rndf = fndrnd(fd);
        if (!rndf)
            {RTFS_PRINT_STRING_1(USTRING_TSTSH_11,PRFLG_NL);} /* "Cant find file" */
        else
        {
            agv++;
            recno = rtfs_atoi(*agv);
            foff = (long) recno * rndf->reclen;

            if (foff !=(seekret = po_lseek(fd, foff, PSEEK_SET ) ))
            {
                 DISPLAY_ERRNO("po_lseek")
                 RTFS_PRINT_STRING_1(USTRING_TSTSH_12, PRFLG_NL); /* "Seek operation failed " */
            }
            else
                return (1);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_13,PRFLG_NL); /* "Usage: SEEK fd recordno" */
    return (0);
}

/* READ fd */
int doread(int agc, byte **agv)                                     /*__fn__*/
{
    PCFD fd;
    RNDFILE *rndf;
    int res;

    if (agc == 1)
    {
        fd = (PCFD) rtfs_atoi(*agv);
        rndf = fndrnd(  fd );
        if (!rndf)
            {RTFS_PRINT_STRING_1(USTRING_TSTSH_14,PRFLG_NL);} /* "Cant find file" */
        else
        {
            if ( (res = po_read(fd,(byte*)rndf->buff,(word)rndf->reclen)) != rndf->reclen)
            {
                DISPLAY_ERRNO("po_read")
                RTFS_PRINT_STRING_1(USTRING_TSTSH_15, PRFLG_NL); /* "Read operation failed " */
            }
            else
            {
                RTFS_PRINT_STRING_2(USTRING_SYS_NULL, rndf->buff,PRFLG_NL);
                return (1);
            }
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_16,PRFLG_NL); /* "Usage: READ fd" */
    return (0);
}

/* WRITE fd "data" */
int dowrite(int agc, byte **agv)                                /*__fn__*/
{
    PCFD fd;
    RNDFILE *rndf;
    int res;

    if (agc == 2)
    {
        fd = rtfs_atoi(*agv++);
        rndf = fndrnd(  fd );
        if (!rndf)
            {RTFS_PRINT_STRING_1(USTRING_TSTSH_17,PRFLG_NL);} /* "Cant find file" */
        else
        {
            pc_cppad((byte*)rndf->buff,(byte*) *agv,(int) rndf->reclen);
            rndf->buff[rndf->reclen] = CS_OP_ASCII('\0');
            if ( (res = po_write(fd,(byte*)rndf->buff,(word)rndf->reclen)) != rndf->reclen)
            {
                DISPLAY_ERRNO("po_write")
                RTFS_PRINT_STRING_1(USTRING_TSTSH_18, PRFLG_NL); /* "Write operation failed " */
            }
            else
            {
                return (1);
            }
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_19,PRFLG_NL); /* "Usage: WRITE fd data " */
    return (0);
}

/* LSTOPEN */
int dolstop(int agc, byte **agv)                                /*__fn__*/
{
    int i;
    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *)agv);

    for (i = 0 ; i < MAXRND; i++)
    {
        if (rnds[i].fd != -1)
        {
            RTFS_PRINT_LONG_1((dword)rnds[i].fd, 0);
            RTFS_PRINT_STRING_2(USTRING_TSTSH_30, rnds[i].name, PRFLG_NL);
        }
    }
    return (1);
}

/* MKDIR PATH */
int domkdir(int agc, byte **agv)                            /*__fn__*/
{
    if (agc == 1)
    {
        if (pc_mkdir(*agv))
             return (1);
        else
        {
            DISPLAY_ERRNO("pc_mkdir")
            return(0);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_20,PRFLG_NL); /* "Usage: MKDIR D:PATH" */
    return (0);
}

/* RMDIR PATH */
int dormdir(int agc, byte **agv)                                    /*__fn__*/
{

    if (agc == 1)
    {
        if (pc_rmdir(*agv))
             return (1);
        else
        {
            DISPLAY_ERRNO("pc_rmdir")
            return(0);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_21,PRFLG_NL); /* "Usage: RMDIR D:PATH\n" */
    return (0);
}

/* DELTREE PATH */
int dodeltree(int agc, byte **agv)                                    /*__fn__*/
{

    if (agc == 1)
    {
        if (pc_deltree(*agv))
             return (1);
        else
        {
            DISPLAY_ERRNO("pc_deltree")
            return(0);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_22,PRFLG_NL); /* "Usage: DELTREE D:PATH" */
    return (0);
}

/* DELETE PATH */
byte rmfil[EMAXPATH_BYTES];
byte rmpath[EMAXPATH_BYTES];
byte tfile[EMAXPATH_BYTES];
byte _rmpath[EMAXPATH_BYTES];
int dorm(int agc, byte **agv)                                       /*__fn__*/
{
    DSTAT statobj;

    if (agc == 1)
    {
        /* Get the path */
        rtfs_cs_strcpy(&rmfil[0],*agv);

        if (pc_gfirst(&statobj, &rmfil[0]))
        {
            do
            {
                /* Construct the path to the current entry ASCII 8.3 */
                pc_ascii_mfile((byte*) &tfile[0],(byte*) &statobj.fname[0], (byte*) &statobj.fext[0]);
                pc_mpath((byte*)&_rmpath[0],(byte*) &statobj.path[0],(byte*)&tfile[0] );
                /* Convert to native character set if needed */
                CS_OP_ASCII_TO_CS_STR(&rmpath[0], &_rmpath[0]);
                if (!pc_isdir(rmpath) && !pc_isvol(rmpath))
                {
                    RTFS_PRINT_STRING_2(USTRING_TSTSH_23,rmpath,PRFLG_NL); /* "deleting  --> " */
                    if (!pc_unlink( rmpath ) )
                    {
                        DISPLAY_ERRNO("pc_unlink")
                        RTFS_PRINT_STRING_2(USTRING_TSTSH_24,rmpath,PRFLG_NL); /* "Can not delete: " */
                    }
                }
            }  while(pc_gnext(&statobj));
            DISPLAY_ERRNO("pc_gnext: PENOENT IS OKAY")
            pc_gdone(&statobj);
        }
        else
        {
            DISPLAY_ERRNO("pc_gfirst")
        }
        return(1);
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_25,PRFLG_NL); /* "Usage: DELETE D:PATH" */
    return(0);
}

/* RENAME PATH NEWNAME */
int domv(int agc, byte **agv)                                      /*__fn__*/
{
    if (agc == 2)
    {
        if (pc_mv(*agv , *(agv+1)))
             return (1);
        else
        {
            DISPLAY_ERRNO("pc_mv")
            return(0);
        }
    }

    RTFS_PRINT_STRING_1(USTRING_TSTSH_26,PRFLG_NL); /* "Usage:  RENAME PATH NEWNAME" */
    return (0);
}


/* CD PATH */
int docwd(int agc, byte **agv)                                      /*__fn__*/
{
    byte lbuff[EMAXPATH_BYTES];

    if (agc == 1)
    {
        if (pc_set_cwd(*agv))
            return(1);
        else
        {
            DISPLAY_ERRNO("pc_set_cwd")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_27,PRFLG_NL); /* "Set cwd failed" */
            return(0);
        }
    }
    else
    {
        if (pc_pwd(rtfs_strtab_user_string(USTRING_SYS_NULL),lbuff))
        {
            RTFS_PRINT_STRING_2(USTRING_SYS_NULL,lbuff,PRFLG_NL);
            return(1);
        }
        else
        {
            DISPLAY_ERRNO("pc_pwd")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_28,PRFLG_NL); /* "Get cwd failed" */
            return(0);
        }
    }
}

/* DIR PATH */
int dols(int agc, byte **agv)                                      /*__fn__*/
{
    int fcount;
    int doit, addwild;
    byte *ppath,*p;
    dword blocks_total, blocks_free;
    dword nfree;
    byte null_str[2];
    ppath = 0;

    addwild = 0;
    if (agc == 1)
    {
        ppath = *agv;
        p = ppath;
        /* If the second char is ':' and the third is '\0' then we have
           D: and we will convert to D:*.* */
        CS_OP_INC_PTR(p);
        if (CS_OP_CMP_ASCII(p,':'))
        {
            CS_OP_INC_PTR(p);
            if (CS_OP_IS_EOS(p))
                addwild = 1;
        }
    }
    else
    {
        null_str[0] = null_str[1] = 0; /* Works for all char sets */
        /* get the working dir of the default dir */
        if (!pc_pwd(null_str,(byte *)shell_buf))
        {
            RTFS_PRINT_STRING_1(USTRING_TSTSH_29,PRFLG_NL); /* "PWD Failed " */
            return(1);
        }
        ppath = (byte *)shell_buf;
        p = ppath;

        /* If not the root add a \\ */
        doit = 1; /* Add \\ if true */
        if (CS_OP_CMP_ASCII(p,'\\'))
        {
            CS_OP_INC_PTR(p);
            if (CS_OP_IS_EOS(p))
                doit = 0;
        }
        if (doit)
        {
            p = CS_OP_GOTO_EOS(p);
            CS_OP_ASSIGN_ASCII(p,'\\');
            CS_OP_INC_PTR(p);
            CS_OP_TERM_STRING(p);
        }
        addwild = 1;
    }
    if (addwild)
    {
        p = ppath;
        p = CS_OP_GOTO_EOS(p);
        /* Now tack on *.* */
        CS_OP_ASSIGN_ASCII(p,'*');
        CS_OP_INC_PTR(p);
        /* Add .* for non vfat */
        CS_OP_ASSIGN_ASCII(p,'.');
        CS_OP_INC_PTR(p);
        CS_OP_ASSIGN_ASCII(p,'*');
        CS_OP_INC_PTR(p);
        CS_OP_TERM_STRING(p);
    }


    /* Now do the dir */
    fcount = pc_seedir(ppath);

   /* And print the bytes remaining */
    nfree = pc_free(ppath, &blocks_total, &blocks_free);
/* printf( "       %-6d File(s)   %-8ld KBytes free\n", fcount, blocks_free/2 ); */

    RTFS_PRINT_STRING_1(USTRING_TSTSH_30, 0); /* "       " */
    RTFS_PRINT_LONG_1((dword) fcount, 0);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_31, 0); /* " File(s) " */
    RTFS_PRINT_LONG_1((dword) blocks_free, 0);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_32, PRFLG_NL); /* " Blocks Free " */
    return(1);
}

/* List directory, and return number of matching file */
int pc_seedir(byte *path)                                          /*__fn__*/
{
    int fcount = 0;
    DSTAT statobj;
    byte display_buffer[100];
    /* Get the first match */
    if (pc_gfirst(&statobj, path))
    {
        for (;;)
        {
            fcount++;

            rtfs_print_format_dir(display_buffer, &statobj);

             /* Get the next */
            if (!pc_gnext(&statobj))
                break;

        }
        DISPLAY_ERRNO("pc_next")
        /* Call gdone to free up internal resources used by statobj */
        pc_gdone(&statobj);
    }
    else
    {
        DISPLAY_ERRNO("pc_gfirst")
    }
    return(fcount);
}


int doregress(int agc, byte **agv)
{
    if (agc == 1)
    {
        pc_regression_test(*agv, TRUE);
        return(1);
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_110,PRFLG_NL); /*  "Usage: REGRESSTEST D:" */
        return(0);
    }
}


/* CAT PATH */
int docat(int agc, byte **agv)                                  /*__fn__*/
{
    PCFD fd;
    int nread;


    if (agc == 1)
    {
        if ((fd = po_open(*agv, (word)(PO_BINARY|PO_RDONLY|PO_BUFFERED),(word) (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_33, *agv,PRFLG_NL); /* "Cant open " */
            return(0);
        }
        else
        {
            do
            {

                nread = po_read(fd,(byte*)shell_buf,1);
                if (nread > 0)
                {
                    shell_buf[1] = '\0';
                    RTFS_PRINT_STRING_2(USTRING_SYS_NULL,shell_buf,0);
                }
                if (nread < 0)
                {
                    DISPLAY_ERRNO("po_read")
                }
            }while(nread > 0);
            if (po_close(fd) != 0)
            {
                DISPLAY_ERRNO("po_read")
            }
            return(1);
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_34,PRFLG_NL); /* "Usage: CAT PATH" */
        return(0);
    }
}


#if (INCLUDE_CHKDSK)

/*  */
int dochkdsk(int agc, byte **agv)                                  /*__fn__*/
{
int write_chains;
CHKDISK_STATS chkstat;
    if (agc != 2)
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_35,PRFLG_NL); /* "Usage: CHKDSK DRIVE: WRITECHAINS" */
        RTFS_PRINT_STRING_1(USTRING_TSTSH_36,PRFLG_NL); /* "Example:CHKDSK A: 1" */
        RTFS_PRINT_STRING_1(USTRING_TSTSH_37,PRFLG_NL); /* "Runs chkdsk on A: writes lost chains" */
        RTFS_PRINT_STRING_1(USTRING_TSTSH_38,PRFLG_NL); /* "Example:CHKDSK A: 0" */
        RTFS_PRINT_STRING_1(USTRING_TSTSH_39,PRFLG_NL); /* "Runs chkdsk on A: does not write lost chains" */
        return(0);
    }
    write_chains = (int) rtfs_atoi( *(agv+1) );
    pc_check_disk(*agv, &chkstat, 1, write_chains, write_chains);
    return(0);
}
#endif

int dochsize(int agc, byte **agv)                                  /*__fn__*/
{
    PCFD fd;
    long newsize;

    if (agc == 2)
    {
        if ((fd = po_open(*agv, (word)(PO_BINARY|PO_WRONLY),(word) (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_40, *agv ,PRFLG_NL); /* "Cant open file: " */
            return(0);
        }
        newsize = rtfs_atol( *(agv+1) );
        if (po_chsize(fd, newsize) != 0)
        {
            DISPLAY_ERRNO("po_chsize")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_41, PRFLG_NL); /* "Change size function failed" */
        }
        if (po_close(fd) != 0)
        {
            DISPLAY_ERRNO("po_close")
        }
        return(1);
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_42,PRFLG_NL); /* "Usage: CHSIZE PATH NEWSIZE" */
        return(0);
    }
}

/* COPY PATH PATH */
int docopy(int agc, byte **agv)                                 /*__fn__*/
{
    PCFD in_fd;
    PCFD out_fd;
    int nread;
    BOOLEAN forever = TRUE;         /* use this for while(forever) to quiet anal compilers */

    if (agc == 2)
    {
        if ((in_fd = po_open(*agv,(word) (PO_BINARY|PO_RDONLY),(word) (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_43, *agv,PRFLG_NL); /* "Cant open file: " */
            return(0);
        }
        if ((out_fd = po_open(*(agv+1),(word) (PO_BINARY|PO_WRONLY|PO_CREAT),(word) (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_44, *(agv+1),PRFLG_NL); /* "Cant open file" */
            return(0);
        }
        while (forever)
        {
            nread = (word)po_read(in_fd,(byte*)shell_buf, 1024);
            if (nread < 0)
            {
                DISPLAY_ERRNO("po_read")
                break;
            }
            else if (nread > 0)
            {
                if (po_write(out_fd,(byte*)shell_buf,(word)nread) != (int)nread)
                {
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_45,PRFLG_NL); /* "Write failure " */
                    break;
                }
            }
            else
                break;
        }
        po_close(in_fd);
        po_close(out_fd);
        return(1);
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_46,PRFLG_NL); /* "Usage: COPY FROMPATH TOPATH\n" */
        return(0);
    }
}

/* DIFF PATH PATH */
int dodiff(int agc, byte **agv)                                  /*__fn__*/
{
    PCFD in_fd;
    PCFD in_fd1;
    int nread;
    int nread1;
    int i;
    byte buff[256];
    byte buff1[256];


    if (agc == 2)
    {
        if ((in_fd = po_open(*agv, (PO_BINARY|PO_RDONLY), (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_47, *agv,PRFLG_NL); /* "Cant open file: " */
            return(0);
        }
        if ((in_fd1 = po_open(*(agv+1), (PO_BINARY|PO_RDONLY), (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_48, *(agv+1),PRFLG_NL); /* "Cant open file" */
            return(0);
        }
        for (;;)
        {
            nread = po_read(in_fd,(byte*)buff,(word)255);
            if (nread > 0)
            {
                nread1 = po_read(in_fd1,(byte*)buff1,(word)nread);
                if (nread1 != nread)
                {
                    if (nread1 < 0)
                    {
                        DISPLAY_ERRNO("po_read")
                    }
difffail:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_49,PRFLG_NL);     /* "Files are different" */
                    po_close (in_fd);
                    po_close (in_fd);
                    return(0);
                }
                for(i = 0; i < nread; i++)
                {
                    if (buff[i] != buff1[i])
                        goto difffail;
                }
            }
            else
            {
                if (nread < 0)
                {
                    DISPLAY_ERRNO("po_read")
                }
                break;
            }
        }
        nread1 = po_read(in_fd1,(byte*)buff1,(word)nread);
        if (nread1 <= 0)
        {
                if (nread1 < 0)
                {
                    DISPLAY_ERRNO("po_read")
                }
                RTFS_PRINT_STRING_2(USTRING_TSTSH_50,*agv,0); /* "File: " */
                RTFS_PRINT_STRING_2(USTRING_TSTSH_51,*(agv+1),0); /* " And File: " */
                RTFS_PRINT_STRING_1(USTRING_TSTSH_52,PRFLG_NL); /* " are the same" */
        }
        else
        {
            {
                RTFS_PRINT_STRING_2(USTRING_TSTSH_53, *(agv+1), 0); /* "File: " */
                RTFS_PRINT_STRING_2(USTRING_TSTSH_54, *agv, PRFLG_NL); /* " is larger than File: " */
            }
            goto difffail;
        }
        po_close(in_fd);
        po_close(in_fd1);
        return(1);
   }
   else
   {
       RTFS_PRINT_STRING_1(USTRING_TSTSH_55, PRFLG_NL); /* "Usage: DIFF PATH PATH " */
   }
    return (0);
}


/* FILLFILE PATH PATTERN NTIMES */
int dofillfile(int agc, byte **agv)                                 /*__fn__*/
{
    PCFD out_fd;
    byte  workbuf[255];
    word bufflen;
    int ncopies;

    if (agc == 3)
    {
        ncopies = (int) rtfs_atoi( *(agv+2) );
        if (!ncopies)
            ncopies = 1;

        if ((out_fd = po_open(*agv,(word) (PO_BINARY|PO_WRONLY|PO_CREAT),(word) (PS_IWRITE | PS_IREAD) ) ) < 0)
        {
            DISPLAY_ERRNO("po_open")
            RTFS_PRINT_STRING_2(USTRING_TSTSH_56, *agv, PRFLG_NL); /* "Cant open file" */
            return(0);
        }
        rtfs_cs_strcpy(workbuf, *(agv+1));
        rtfs_strcat(workbuf, (byte *) "\r\n");
        bufflen = (word) rtfs_strlen(workbuf);

        while (ncopies--)
        {
            if (po_write(out_fd,(byte*)workbuf,(word)bufflen) != (int)bufflen)
            {
                DISPLAY_ERRNO("po_write")
                RTFS_PRINT_STRING_1(USTRING_TSTSH_57, PRFLG_NL); /* "Write failure" */
                po_close(out_fd);
                return(0);
            }
        }
        po_close(out_fd);
        return(1);
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_58, PRFLG_NL); /* "Usage: FILLFILE PATH PATTERN NTIMES" */
        return(0);
    }
}

/* STAT PATH */
int dostat(int agc, byte **agv)                                    /*__fn__*/
{
ERTFS_STAT st;
byte display_buffer[256];
    if (agc == 1)
    {
        if (pc_stat(*agv, &st)==0)
        {
            rtfs_print_format_stat(display_buffer, &st);
            RTFS_PRINT_STRING_1(USTRING_TSTSH_59, 0); /* "MODE BITS :" */
            if (st.st_mode&S_IFDIR)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_60, 0); /* "S_IFDIR|" */
            if (st.st_mode&S_IFREG)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_61, 0); /* "S_IFREG|" */
            if (st.st_mode&S_IWRITE)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_62, 0); /* "S_IWRITE|" */
            if (st.st_mode&S_IREAD)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_63, 0); /* "S_IREAD\n" */
            RTFS_PRINT_STRING_1(USTRING_SYS_NULL, PRFLG_NL); /* "" */

            return (1);
        }
        else
        {
            DISPLAY_ERRNO("pc_stat")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_65, PRFLG_NL); /* "FSTAT failed" */
            return(0);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_66, PRFLG_NL); /* "Usage: FSTAT D:PATH" */
    return (0);
}
#if (INCLUDE_FAILSAFE_CODE)
int dofsstatus(int agc, byte **agv)
{
struct pro_buffer_stats s;
    if (agc == 1)
    {
        if (!pro_buffer_status(*agv, &s))
        {
            RTFS_PRINT_STRING_1(USTRING_TSTSH_136, PRFLG_NL); /* "pro_buffer_status failed" */
            return -1;
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_137, PRFLG_NL); /* "USAGE: BUFFERSTAT D:" */
        return -1;
    }
    if (s.failsafe_mode == 0)
        RTFS_PRINT_STRING_1(USTRING_TSTSH_118, PRFLG_NL); /* "Failsafe disabled" */
    else if (s.failsafe_mode == 1)
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_119, PRFLG_NL); /* "Failsafe enabled with journaling " */

        RTFS_PRINT_STRING_1(USTRING_TSTSH_147, 0); /* "Failsafe file blocks consumed"  */
        RTFS_PRINT_LONG_1(s.failsafe_blocks_used, PRFLG_NL);

        RTFS_PRINT_STRING_1(USTRING_TSTSH_148, 0); /* "Failsafe file blocks still available" */
        RTFS_PRINT_LONG_1(s.failsafe_blocks_free, PRFLG_NL);
    }
    else if (s.failsafe_mode == 2)
        RTFS_PRINT_STRING_1(USTRING_TSTSH_120, PRFLG_NL); /* "Failsafe enabled with no journaling" */



    RTFS_PRINT_STRING_1(USTRING_TSTSH_121, 0); /* "   Total Number of Block Buffers" */
    RTFS_PRINT_LONG_1(s.total_block_buffers, PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_122, 0); /* "   Pending writes on Block Buffers" */
    RTFS_PRINT_LONG_1(s.block_buffers_pending,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_123, 0); /* "   Block Buffers available" */
    RTFS_PRINT_LONG_1(s.block_buffers_available,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_124, 0); /* "   Block Buffer Cache Hits" */
    RTFS_PRINT_LONG_1(s.block_buffers_cache_hits,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_125, 0); /* "   Block Buffer Cache Misses" */
    RTFS_PRINT_LONG_1(s.block_buffers_cache_misses,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_126, 0); /* "   Block Buffer Low Water" */
    RTFS_PRINT_LONG_1(s.block_buffers_low,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_127, 0); /* "   Block Allocation failures" */
    RTFS_PRINT_LONG_1(s.block_buffers_fail,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_128, 0); /* "   Total Number of FAT Buffers" */
    RTFS_PRINT_LONG_1(s.total_fat_buffers,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_129, 0); /* "   FAT Buffers never used" */
    RTFS_PRINT_LONG_1(s.fat_buffers_free,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_130, 0); /* "   FAT Buffer Primary Cache Hits" */
    RTFS_PRINT_LONG_1(s.fat_buffer_primary_cache_hits,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_131, 0); /* "   FAT Buffer Secondary Cache Hits" */
    RTFS_PRINT_LONG_1(s.fat_buffer_secondary_cache_hits,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_132, 0); /* "   FAT Buffer Secondary Cache Loads" */
    RTFS_PRINT_LONG_1(s.fat_buffer_cache_loads,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_133, 0); /* "   FAT Buffer Secondary Cache Swaps" */
    RTFS_PRINT_LONG_1(s.fat_buffer_cache_swaps,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_134, 0); /* "   Pending writes on FAT Buffers" */
    RTFS_PRINT_LONG_1(s.fat_buffers_pending,PRFLG_NL);
    RTFS_PRINT_STRING_1(USTRING_TSTSH_135, 0); /* "   FAT Buffers available" */
    RTFS_PRINT_LONG_1(s.fat_buffers_available,PRFLG_NL);
    return (0);
}



int fs_test(byte *drivename);

int dofstest(int agc, byte **agv)
{
    if (agc != 1)
        return (-1);
    else
        return (fs_test(*agv));
}

/* Reserve some storage for block mapping structure. See the
   description of fscontext.blockmap_size below for a discussion
   on block maps */
FAILSAFECONTEXT fscontext;
#define BLOCKMAPSIZE 128
FSBLOCKMAP failsafe_blockmap_array[BLOCKMAPSIZE];

void app_failsafe_init(byte *drive_name)
{
   rtfs_memset((void *) &fscontext, 0, sizeof(fscontext));
    /* Initial setting.
         autorestore, autocommit and autorecover options all disabled */
    fscontext.configuration_flags = 0;
    /* Select AUTORESTORE. - The volume is autorestored if needed at mount time
      . If restore is needed but the failsafe file is damaged the mount fails
        unless FS_MODE_AUTORECOVER is also enabled */
    fscontext.configuration_flags |= FS_MODE_AUTORESTORE;
    /* Select AUTORECOVER. - If autorestore failed because the failsafe file
      is damaged skip the restore state and proceed with the mount */
    fscontext.configuration_flags |= FS_MODE_AUTORECOVER;
    /* Enable AUTOCOMMIT mode. Force ERTFS to perform the FailSafe commit
       internally before the return from API calls that may have changed
       volume */
    fscontext.configuration_flags |= FS_MODE_AUTOCOMMIT;
    /* Optional setting - If the user_journal_size is set prior to the
       call to pro_failsafe_init() the FailSafe file space is limitted to
       this size. Otherwise the Failsafe file size is set to the size of
       the FAT plus CFG_NUM_JOURNAL_BLOCKS (prfailsf.h) blocks.
       The default setting are purposely conservative. One block must be
       available for each FAT block and directory block that will be
       changed between calls to pro_failsafe_commit().
       The following line, if enabled, will fix the failsafe journal size
       to 64K. This would be appropriate for example if the journal file
       was being held in 64K of nvram.  */
    /* fscontext.user_journal_size = 128; */    /* In blocks */
    /* Provide buffer space for failsafe to cache indexing information
       pertaining to journaling activities. Without adequate caching
       failsafe will still perform as specified but with lowered
       performance. Each blockmap element required approximately
       32 bytes, for optimal performance one element should be
       available for each FAT block and directory block that will be
       changed between calls to pro_failsafe_commit(). In this example
       we choose 128 entries. This is very a conservative setting that will
       allow failsafe to use caching in a session with up to 128
       FAT and directory blocks changed.
       Note that performance will degrade drastically if the number of
       blocks modified exceeds the size of the block map.
       If you have a need to minimize resources you may reduce blockmap_size
       and user_journal_size. To help determine appropriate values you may
       call pro_buffer_status(), the failsafe_blocks_used field in the status
       structure will contain the number of these resources used.
       In order to gauge the worst case usage, pro_buffer_status() should be
       called immediately prior to calling pro_failsafe_commit()
       */
    fscontext.blockmap_size = BLOCKMAPSIZE;
    fscontext.blockmap_freelist = &failsafe_blockmap_array[0];

    if (!pro_failsafe_init(drive_name, &fscontext))
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_114, PRFLG_NL); /* "Failsafe init failed\n" */
        if (get_errno() == PEFSREINIT)
        {
            /* "Failsafe already running on a drive\n" */
            RTFS_PRINT_STRING_1(USTRING_TSTSH_115, PRFLG_NL);
            /* "Command shell only supports one failsafe session\n" */
            RTFS_PRINT_STRING_1(USTRING_TSTSH_116, PRFLG_NL);
        }
    }

}

int dofsinit(int agc, byte **agv)
{
    if (agc == 1)
         app_failsafe_init(*agv);
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_111, PRFLG_NL); /* "Usage: FAILSAFEINIT D:" */
    }
    return(0);
}
int dofscommit(int agc, byte **agv)
{
    if (agc == 1)
    {
        if (!pro_failsafe_commit(*agv))
        {
            RTFS_PRINT_STRING_1(USTRING_TSTSH_113, PRFLG_NL); /* "Failsafe commit failed\n" */
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_112, PRFLG_NL); /* "Usage: FAILCOMMIT D:" */
    }
    return(0);
}
int dofsrestore(int agc, byte **agv)
{
int rval;

    if (agc == 1)
    {
        rval = pro_failsafe_restore(*agv,0,FALSE,FALSE);
        switch (rval)
        {
            case FS_STATUS_OK:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_138, PRFLG_NL); /* "FAILSAFE: Volume up to date, no restore required" */
            break;
            case FS_STATUS_NO_JOURNAL:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_139, PRFLG_NL); /* "FAILSAFE: no journal present" */
            break;
            case FS_STATUS_BAD_JOURNAL:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_140, PRFLG_NL); /* "FAILSAFE: bad journal file present" */
            break;
            case FS_STATUS_BAD_CHECKSUM:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_141, PRFLG_NL); /* "FAILSAFE: journal file has checksum error" */
            break;
            case FS_STATUS_IO_ERROR:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_142, PRFLG_NL); /* "FAILSAFE: IO error during restore" */
            break;
            case FS_STATUS_RESTORED:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_144, PRFLG_NL); /* "FAILSAFE: Volume was restored sucessfully" */
            break;
            case FS_STATUS_MUST_RESTORE:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_145, PRFLG_NL); /* "FAILSAFE: Restore required" */
            break;
            default:
                RTFS_PRINT_STRING_1(USTRING_TSTSH_146, 0); /* "FAILSAFE: Restore unknown return code" */
                RTFS_PRINT_LONG_1((dword)rval, PRFLG_NL);
            break;
        }
        if (rval == FS_STATUS_MUST_RESTORE)
        {
            rval = pro_failsafe_restore(*agv,0,TRUE,FALSE);
            switch (rval)
            {
                case FS_STATUS_BAD_JOURNAL:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_140, PRFLG_NL); /* "FAILSAFE: bad journal file present" */
                break;
                case FS_STATUS_BAD_CHECKSUM:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_141, PRFLG_NL); /* "FAILSAFE: journal file has checksum error" */
                break;
                case FS_STATUS_IO_ERROR:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_142, PRFLG_NL); /* "FAILSAFE: IO error during restore" */
                break;
                case FS_STATUS_RESTORED:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_144, PRFLG_NL); /* "FAILSAFE: Volume was restored sucessfully" */
                break;
                case FS_STATUS_MUST_RESTORE:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_145, PRFLG_NL); /* "FAILSAFE: Restore required" */
                break;
                default:
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_146, 0); /* "FAILSAFE: Restore unknown return code" */
                    RTFS_PRINT_LONG_1((dword)rval, PRFLG_NL);
                break;
            }
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_111, PRFLG_NL); /* "Usage: FAILSAFERESTORE D:" */
    }
    return(0);
}
#endif

/* FORMAT D:*/
int doformat(int agc, byte **agv)                                    /*__fn__*/
{
byte buf[10];
byte working_buffer[100];
DDRIVE *pdr;
int driveno;
dword partition_list[3];
byte path[10];
DEV_GEOMETRY geometry;

    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *)agv);


    /* "Enter the drive to format as A:, B: etc " */
    rtfs_print_prompt_user(UPROMPT_TSTSH3, path);
    pdr = 0;
    driveno = pc_parse_raw_drive(path);
    if (driveno != -1)
        pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr)
    {
inval:
        /*  "Invalid drive selection to format, press return" */
        rtfs_print_prompt_user(UPROMPT_TSTSH4, working_buffer);
        return(-1);
    }

    if ( !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
    {
        goto inval;
    }

    OS_CLAIM_LOGDRIVE(driveno)
    /* check media and clear change conditions */
    if (!check_drive_number_present(driveno))
    {
        /* "Format - check media failed. Press return" */
        rtfs_print_prompt_user(UPROMPT_TSTSH5, working_buffer);
        goto return_error;
    }

    /* This must be called before calling the later routines */
    if (!pc_get_media_parms(path, &geometry))
    {
        /* "Format: get media geometry failed. Press return" */
        rtfs_print_prompt_user(UPROMPT_TSTSH6, working_buffer);
        goto return_error;
    }

    /* Call the low level media format. Do not do this if formatting a
       volume that is the second partition on the drive */
    /* "Format: Press Y to format media " */
    rtfs_print_prompt_user(UPROMPT_TSTSH7, buf);
    if (tstsh_is_yes(buf))
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_67, PRFLG_NL); /* "Calling media format" */
        if (!pc_format_media(path, &geometry))
        {
            DISPLAY_ERRNO("pc_format_media")
           /* "Format: Media format failed. Press return" */
           rtfs_print_prompt_user(UPROMPT_TSTSH8, working_buffer);
           goto return_error;
        }
    }

    /* Partition the drive if it needs it */
    if ( pdr->drive_flags&DRIVE_FLAGS_PARTITIONED )
    {
        /* "Format: Press Y to partition media " */
        rtfs_print_prompt_user(UPROMPT_TSTSH9, buf);
        if (tstsh_is_yes(buf))
        {
            if (geometry.dev_geometry_lbas)
            {
                do
                {
                    /* "Format: Press Y to USE LBA formatting, N to use CHS " */
                    rtfs_print_prompt_user(UPROMPT_TSTSH10, buf);
                }
                while (!tstsh_is_yes(buf) && !tstsh_is_no(buf));

                if (tstsh_is_no(buf))
                {
                    geometry.dev_geometry_lbas = 0;
                }
            }

            if (geometry.dev_geometry_lbas)
            {
               RTFS_PRINT_STRING_1(USTRING_TSTSH_68, 0); /* "The drive is contains this many logical blocks " */
               RTFS_PRINT_LONG_1((dword) geometry.dev_geometry_lbas, PRFLG_NL);
               /* "Format: Select the number of lbas for the first partition :" */
               rtfs_print_prompt_user(UPROMPT_TSTSH11, working_buffer);
               partition_list[0]  = (dword)rtfs_atol(working_buffer);
               partition_list[1]  = 0;

               if (partition_list[0] != geometry.dev_geometry_lbas)
               {
                   RTFS_PRINT_STRING_1(USTRING_TSTSH_69, 0); /* "This many logical blocks remain" */
                   RTFS_PRINT_LONG_1((dword) (geometry.dev_geometry_lbas - partition_list[0]), PRFLG_NL);
                   /* "Format: Select the number of lbas for the second partition :" */
                   rtfs_print_prompt_user(UPROMPT_TSTSH12, working_buffer);
                   partition_list[1]  = (dword)rtfs_atol(working_buffer);
                   partition_list[2]  = 0;
               }
               if ((partition_list[0] == 0) || ((dword)(partition_list[0]+partition_list[1])  > geometry.dev_geometry_lbas))
               {
                   /* "Format: Bad input for partition values. Press return" */
                   rtfs_print_prompt_user(UPROMPT_TSTSH13, working_buffer);
                   goto return_error;
                }
            }
            else
            {
                RTFS_PRINT_STRING_1(USTRING_TSTSH_70, 0); /* "The drive contains this many cylinders" */
                RTFS_PRINT_LONG_1((dword) geometry.dev_geometry_cylinders, PRFLG_NL);
                /* "Format: Select the number of cyls for the first partition :" */
                rtfs_print_prompt_user(UPROMPT_TSTSH14, working_buffer);
                partition_list[0]  = (word)rtfs_atoi(working_buffer);
                partition_list[1]  = 0;

                if (partition_list[0] != geometry.dev_geometry_cylinders)
                {
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_71, 0); /* "There are this many cylinders remainng" */
                    RTFS_PRINT_LONG_1((dword) geometry.dev_geometry_cylinders - partition_list[0], PRFLG_NL);
                    /* "Format: Select the number of cyls for the second partition :" */
                    rtfs_print_prompt_user(UPROMPT_TSTSH15, working_buffer);
                    partition_list[1]  = (dword)rtfs_atol(working_buffer);
                    partition_list[2]  = 0;
               }
               if ((partition_list[0] == 0) || ((dword)(partition_list[0]+partition_list[1])  > geometry.dev_geometry_cylinders))
               {
                   /* "Format: Bad input for partition values. Press return" */
                   rtfs_print_prompt_user(UPROMPT_TSTSH13, working_buffer);
                   goto return_error;
                }
            }
            if (!pc_partition_media(path, &geometry, &partition_list[0]))
            {
               /* "Format: Media partition failed. Press return" */
               rtfs_print_prompt_user(UPROMPT_TSTSH16, working_buffer);
               goto return_error;
            }
        }
    }

    /* Put the DOS format */
    /* "Format: Press Y to format the volume " */
    rtfs_print_prompt_user(UPROMPT_TSTSH17, buf);
    if (tstsh_is_yes(buf))
    {
        if (!pc_format_volume(path, &geometry))
        {
           /* "Format: Format volume failed. Press return" */
           rtfs_print_prompt_user(UPROMPT_TSTSH18, working_buffer);
           goto return_error;
        }
    }
    OS_RELEASE_LOGDRIVE(driveno)
    return (0);
return_error:
    OS_RELEASE_LOGDRIVE(driveno)
    return(-1);
}


/* GETATTR PATH*/
int dogetattr(int agc, byte **agv)                                    /*__fn__*/
{
byte attr;

    if (agc == 1)
    {
        if (pc_get_attributes(*agv, &attr))
        {
            RTFS_PRINT_STRING_1(USTRING_TSTSH_72,0); /* "Attributes: " */
            if (attr & ARDONLY)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_73,0); /* "ARDONLY|" */
            if (attr & AHIDDEN)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_74,0); /* "AHIDDEN|" */
            if (attr & ASYSTEM)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_75,0); /* "ASYSTEM|" */
            if (attr & AVOLUME)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_76,0); /* "AVOLUME|" */
            if (attr & ADIRENT)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_77,0); /* "ADIRENT|" */
            if (attr & ARCHIVE)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_78,0); /* "ARCHIVE|" */
            if (attr == ANORMAL)
                RTFS_PRINT_STRING_1(USTRING_TSTSH_79,0); /* "NORMAL FILE (No bits set)" */
            RTFS_PRINT_STRING_1(USTRING_SYS_NULL, PRFLG_NL); /* "" */
            return(0);
        }
        else
        {
            DISPLAY_ERRNO("pc_get_attributes")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_81, PRFLG_NL); /* "get attributes failed" */
            return(0);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_82, PRFLG_NL); /* "Usage: GETATTR D:PATH" */
    return (0);
}

/* GETATTR PATH*/
int dosetattr(int agc, byte **agv)                                    /*__fn__*/
{
byte attr;

    attr = 0;
    if (agc == 2)
    {
        if (!pc_get_attributes(*agv, &attr))
        {
            DISPLAY_ERRNO("pc_get_attributes")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_83, PRFLG_NL); /* "Can not get attributes" */
            return(0);
        }
        attr &= ~(ARDONLY|AHIDDEN|ASYSTEM|ARCHIVE);
        if (rtfs_cs_strcmp(*(agv+1),rtfs_strtab_user_string(USTRING_TSTSH_84))==0) /* "RDONLY" */
            attr |= ARDONLY;
        else if (rtfs_cs_strcmp(*(agv+1),rtfs_strtab_user_string(USTRING_TSTSH_85))==0) /* "HIDDEN" */
            attr |= AHIDDEN;
        else if (rtfs_cs_strcmp(*(agv+1),rtfs_strtab_user_string(USTRING_TSTSH_86))==0) /* "SYSTEM" */
            attr |= ASYSTEM;
        else if (rtfs_cs_strcmp(*(agv+1),rtfs_strtab_user_string(USTRING_TSTSH_87))==0) /* "ARCHIVE" */
            attr |= ARCHIVE;
        else if (rtfs_cs_strcmp(*(agv+1),rtfs_strtab_user_string(USTRING_TSTSH_88))==0) /* "NORMAL" */
            attr =  ANORMAL;
        else
            goto usage;

        if (pc_set_attributes(*agv, attr))
            return(0);
        else
        {
            DISPLAY_ERRNO("pc_set_attributes")
            RTFS_PRINT_STRING_1(USTRING_TSTSH_89, PRFLG_NL); /* "Set attributes failed" */
            return(0);
        }
    }
usage:
    RTFS_PRINT_STRING_1(USTRING_TSTSH_90, PRFLG_NL); /* "Usage: SETATTR D:PATH RDONLY|HIDDEN|SYSTEM|ARCHIVE|NORMAL" */

    return (0);
}

/* DEVINFO */
int dodevinfo(int agc, byte **agv)                                /*__fn__*/
{
    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *)agv);

    print_device_names();

    return (1);
}



#if (INCLUDE_CDROM)

/* DSKOPEN PATH */
int cddodskop(int agc, byte **agv)                                    /*__fn__*/
{
    if (agc == 1)
    {
        if (cd_dskopen(*agv))
        {
            cddoselect(agc, agv);
            return(1);
        }
        else
        {
            RTFS_PRINT_STRING_2(USTRING_TSTSH_91,*agv, PRFLG_NL); /* "Can not open disk %s\n" */
            return(0);
        }
    }
    RTFS_PRINT_STRING_1(USTRING_TSTSH_92, PRFLG_NL); /* "Usage: DSKOPEN D:" */
    return(0);
}


/* DSKCLOSE PATH */
int cddodskcl(int agc, byte **agv)                                /*__fn__*/
{
    if (agc == 1)
    {
        cd_dskclose(*agv);
    }
    else
        RTFS_PRINT_STRING_1(USTRING_TSTSH_93, PRFLG_NL); /* "Usage: DSKCLOSE D: " */
    return(1);
 }

/* DSKSEL PATH */
int cddoselect(int agc, byte **agv)                                   /*__fn__*/
{

    if (agc == 1)
    {
        if (cd_set_default_drive(*agv))
            return(1);
        else
        {
            RTFS_PRINT_STRING_1(USTRING_TSTSH_94, PRFLG_NL); /* "cd_set_default failed" */
            return(0);
        }
    }
    else
    {
         RTFS_PRINT_STRING_1(USTRING_TSTSH_95, PRFLG_NL); /* "Usage: DSKSELECT D: " */
         return(0);
    }
}

/* CD PATH */
int cddocwd(int agc, byte **agv)                                      /*__fn__*/
{
byte lbuf[255];
    if (agc == 1)
    {
        if (cd_scwd(*agv))
            return(1);
        else
            return(0);
    }
    else
    {
        lbuf[0] = '\0'; /* CDFS */
        if (cd_gcwd(lbuf))
        {
            RTFS_PRINT_STRING_2(USTRING_SYS_NULL, lbuf, PRFLG_NL);
            return (1);
        }
        else
        {
            RTFS_PRINT_STRING_1(USTRING_TSTSH_96, PRFLG_NL); /* "path error" */
            return(0);
        }
    }
}


/* DIR PATH */
int cddols(int agc, byte **agv)                                      /*__fn__*/
{
    byte *dirstr;
    CD_DSTAT dirent;
    byte lbuf[255];

    if (agc==1)
        rtfs_cs_strcpy(lbuf, *agv);
    else
        rtfs_cs_strcpy(lbuf, (byte*)"*.*"); /* CDFS */

    if (!cd_gfirst(&dirent,lbuf))
        return(0);
    do
    {
        if (dirent.dir.file_flags & FF_DIRECTORY)
            dirstr = (byte *)"<DIR>";
        else
            dirstr = (byte *)"     ";

        if ((dirent.dir.len_fi == 1) && (dirent.dir.file_id[0] == 0))
        {
            dirent.dir.file_id[0] = '.'; /* CDFS */
            dirent.dir.file_id[1] = 0;
        }
        if ((dirent.dir.len_fi == 1) && (dirent.dir.file_id[0] == 1))
        {
            dirent.dir.file_id[0] = '.';/* CDFS */
            dirent.dir.file_id[1] = '.';/* CDFS */
            dirent.dir.file_id[2] = 0;
        }

        printf( "%-15.15s ",dirent.dir.file_id); /* CDFS */
        printf( "%-5.5s ",  dirstr);
        printf( "%7ld ",    dirent.dir.data_len);
        printf( "%02d-",    dirent.dir.record_time.month);
        printf( "%02d-",    dirent.dir.record_time.day);
        printf( "%02d ",    dirent.dir.record_time.year);
        printf( "%02d:",    dirent.dir.record_time.hour);
        printf( "%02d:",    dirent.dir.record_time.min);
        printf( "%02d\n",   dirent.dir.record_time.sec); /* CDFS */

    } while (cd_gnext(&dirent,lbuf));

    cd_gdone(&dirent);
    return(1);
}

/* CAT PATH */
int cddocat(int agc, byte **agv)                                  /*__fn__*/
{
    CDFD fd;
    int n_read;
    byte buff[42];
    BOOLEAN forever = TRUE;         /* use this for while(forever) to quiet anal compilers */


    if (agc == 1)
    {
        fd=cd_open(*agv);

        if (fd < 0)
        {
            RTFS_PRINT_STRING_2(USTRING_TSTSH_97, *agv, PRFLG_NL); /* "Cant open File" */
            return(0);
        }
        else
        {
            while (forever)
            {
                n_read = cd_read(fd, (byte *)buff, 40);
                if (n_read > 0)
                {
                    buff[n_read] = '\0'; /* CDFS */
                    RTFS_PRINT_STRING_2(USTRING_SYS_NULL, buff, PRFLG_NL);
                }
                else
                    break;
            }
            cd_close(fd);
            return(1);
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_98, PRFLG_NL); /* "Usage: CAT PATH" */
        return(0);
    }
}

#define ZERO 0
#if (ZERO)
/* READ PATH */
int cddoread(int agc, byte **agv)                                  /*__fn__*/
{
    CDFD fd;
    word n_read;
    BOOLEAN forever = TRUE;         /* use this for while(forever) to quiet anal compilers */


    if (agc == 1)
    {
        fd=cd_open(*agv);

        if (fd < 0)
        {
            RTFS_PRINT_STRING_2(USTRING_TSTSH_99, *agv, PRFLG_NL); /* "Cant open :" */
            return(0);
        }
        else
        {
            while (forever)
            {
                n_read = cd_read(fd, (byte *)read_buf, (word) 16384);
                if ( (n_read != 0) && (n_read <= 16384)  )
                {
                    RTFS_PRINT_STRING_1(USTRING_TSTSH_100,0); /* "." */
                }
                else
                {
                    RTFS_PRINT_STRING_1(USTRING_SYS_NULL, PRFLG_NL); /* "" */
                    break;
                }
            }
            cd_close(fd);
            return(1);
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_102, PRFLG_NL); /* "Usage: READ PATH" */
        return(0);
    }
}



/* COPY CDFILE DOSFILE */
int cddocopy(int agc, byte **agv)                                  /*__fn__*/
{
    BOOLEAN is_wild = FALSE;
    CD_DSTAT dirent;
    byte *p;
    byte *p2;
    byte *last_s;
    byte tofile[255];
    byte from_dir[255];
    byte from_file[255];

    from_dir[0] = '\0'; /* CDFS */

    if (agc == 2)
    {

        /* See if we have a wild card */
        p = *agv;
        while (*p)
            {if (*p == '*' || *p == '?'){is_wild = TRUE; break;} p++;}/* CDFS */
        if (is_wild)
        {
            /* Copy the source path into from dir. Remember the location just
               past the last \ character. Later on we cat the file to it to
               create the source file name. If there is no '\' this will all
               be harmless */
            p = *agv;
            p2 = &from_dir[0];
            last_s = &from_dir[0];
            while (*p)
            {
                *p2++ = *p;
                if ((*p == BACKSLASH) || (*p == ':'))/* CDFS */
                    last_s = p2;
                p++;
            }
            *last_s = '\0';/* CDFS */

            if (!cd_gfirst(&dirent,*agv))
                return(FALSE);
            do
            {
                if (!(dirent.dir.file_flags & FF_DIRECTORY))
                {
                    rtfs_cs_strcpy(tofile, *(agv+1));
                    strcat(tofile, "\\");
                    strcat(tofile, dirent.dir.file_id);
                    rtfs_cs_strcpy(from_file, from_dir);
                    strcat(from_file, dirent.dir.file_id);
                    if (!copy_one_file(tofile,from_file))
                    {
                        cd_gdone(&dirent);
                        return(FALSE);
                    }
                }
            } while (cd_gnext(&dirent,*agv));
            cd_gdone(&dirent);
        }
        else
        {
            if (!copy_one_file(*(agv+1), *agv))
                return(0);
        }
    }
    else
    {
        RTFS_PRINT_STRING_1(USTRING_TSTSH_103, PRFLG_NL); /* "Usage: COPY CDPATH OSPATH" */
        return(0);
    }
    return(1);
}

/* READ PATH */
BOOLEAN copy_one_file(byte *to, byte *from)                             /*__fn__*/
{
    CDFD fd;
    int fi;
    word n_read;
    BOOLEAN forever = TRUE;         /* use this for while(forever) to quiet anal compilers */
    int do_dot;

    fd=cd_open(from);
    if (fd < 0)
    {
        RTFS_PRINT_STRING_2(USTRING_TSTSH_104, from, PRFLG_NL); /* "Cant open " */
        return(FALSE);
    }
    if ( (fi = open(to,O_WRONLY|O_BINARY|O_CREAT|O_TRUNC, S_IREAD | S_IWRITE)) < 0)        /* msc open */
    {
        RTFS_PRINT_STRING_2(USTRING_TSTSH_105,to, PRFLG_NL); /* "Cant creat " */
        cd_close(fd);
        return(FALSE);
    }
    RTFS_PRINT_STRING_1(from, PRFLG_NL);
    do_dot = 2;
    while (forever)
    {

        cd_read(fd, (byte *)read_buf, (word)32768L);
        if((n_read != 0xffff) && (n_read > 0))
        {
            if (!--do_dot)
            {
                RTFS_PRINT_STRING_1(USTRING_TSTSH_106,0); /* "." */
                do_dot = 2;
            }
            if ( (unsigned)write (fi,read_buf,n_read) != n_read)
            {
                RTFS_PRINT_STRING_1(USTRING_SYS_NULL, PRFLG_NL); /* "" */
                RTFS_PRINT_STRING_1(USTRING_TSTSH_108, PRFLG_NL); /* "Write error" */
                close(fi);
                cd_close(fd);
                return (FALSE);
            }
        }
        else
        {
            close(fi);
            cd_close(fd);
            RTFS_PRINT_STRING_1(USTRING_SYS_NULL, PRFLG_NL); /* "" */
            break;
        }
    }
    return(TRUE);
}

#endif /* ZERO */
#endif /* CDFS */


#if (INCLUDE_STRESS_TESTS)


byte filename[1000][30];
byte PattBuff[4096];
struct stat st[1000];
int rand();


void stat_1000( void)
{
    unsigned long i, ret;

    int fd;

    pc_mkdir((byte *)"\\subdir");
    for(i=0;i<1000;i++)
        sprintf(filename[i], "\\subdir\\file%04d.txt", i);
     for(i=0;i<1000;i++)
    {
                sprintf(PattBuff, "%04d", i);
         printf("Creating %s\n", filename[i]);
        if ((fd = po_open(filename[i],(word)(PO_BINARY|PO_WRONLY|PO_CREAT),(word)(PS_IWRITE | PS_IREAD))) >= 0)
        {

            ret = po_write(fd,(byte*)PattBuff,1024);

            po_close(fd);
        }
        else
        {
            printf("Halt:Open failed %s\n", filename[i]);
            for(;;);
        }
        printf("\n");
    }

     for(i=0;i<1000;i++){
         printf("Statting %s\n", filename[i]);
        if (pc_stat(filename[i], &st[i])!=0){
            printf("Halt: Stat failed %s\n", filename[i]);
            for(;;); /* Error */
        }
        printf("\n");
    }

}

void write_big_file()
{
    unsigned long i, j, k, ret;
    dword *dw;
    int fd;

     sprintf((char *)filename[0], "\\subdir\\bigfile.txt");

     printf("Creating %s\n", filename[0]);
     if ((fd = po_open(filename[0],(word)(PO_BINARY|PO_WRONLY|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
     {
         for(i=0;i<2000;i++)
         {
            if ((i & 64) == 0)
                printf("Writing i == %ld\n",i);
            dw = (dword *) PattBuff;
            for (j = 0; j < 1024; j++)
                *dw++ = i*1024 + j;
            ret = po_write(fd,(byte*)PattBuff,4096);
         }
         po_close(fd);
     }
     printf("reading %s\n", filename[0]);
     if ((fd = po_open(filename[0],(word)(PO_BINARY|PO_RDONLY),(word)(PS_IWRITE | PS_IREAD))) >= 0)
     {
         for(i=0;i<2000;i++)
         {
             if ((i & 64) == 0)
                printf("Reading i == %ld\n",i);
            ret = po_read(fd,(byte*)PattBuff,4096);
            if (ret != 4096)
                printf("Read error at i == %ld\n",i);
            else
            {
                dw = (dword *) PattBuff;
                for (j = 0; j < 1024; j++)
                    if (*dw++ != i*1024 + j)
                        printf("Mattch error j== i == %ld %ld\n", j, i);
            }
         }
         printf("seek and reading %s\n", filename[i]);
         for(k=0;k<2000;k++)
         {
             i = (((dword) rand()) % 2000);

             if ((k & 8) == 0)
                printf("Seeking i == %ld\n",i);
            po_lseek(fd, i*4096, PSEEK_SET);
            ret = po_read(fd,(byte*)PattBuff,4096);
            if (ret != 4096)
                printf("Read error at i == %ld\n",i);
            else
            {
                dw = (dword *) PattBuff;
                for (j = 0; j < 1024; j++)
                    if (*dw++ != i*1024 + j)
                        printf("Mattch error j== i == %ld %ld\n", j, i);
            }
         }
     }
     po_close(fd);
}
#define BIGBUFFINTS 10000 /* have tried 8000000 eight million successfully in hostdisk mode. */
int big_buffer[BIGBUFFINTS];

int myrand()
{
int l;
    l = rand(); l*=rand();
    return(l);
}
int test_long_write(int fd)
{
int i, j, ret, pos, nleft, nleft1, nleft2;

    for (j = 0; j < 10; j++)
    {
        pos = myrand()%BIGBUFFINTS;
        nleft2 = BIGBUFFINTS - pos;
        nleft1 = myrand()%nleft2;
        printf("Write Testing %d of 10000 POS %d, NLEFT %d NLEFT2 %d\n",j, pos, nleft1, nleft2);
        nleft = nleft1;
again:
        for(i=0;i<BIGBUFFINTS;i++)
            big_buffer[i] = 0;
        ret = po_lseek(fd, 0, PSEEK_SET);
        if (ret != 0)
        {
            printf("Write test 0: seek returned %d instead of %d\n", ret, 0);
            return(-1);
        }
        ret = po_write(fd,(byte*)big_buffer,(BIGBUFFINTS*sizeof(int)));
        if (ret != (BIGBUFFINTS*sizeof(int)))
        {
            printf("Write test 1: write returned %d instead of (BIGBUFFINTS*sizeof(int))\n", ret);
            return -1;
        }

        ret = po_lseek(fd, pos*4, PSEEK_SET);
        if (ret != pos*4)
        {
            printf("Write test 1: seek returned %d instead of %d\n", ret, pos*4);
            return(-1);
        }
        for(i=0;i<nleft;i++)
            big_buffer[i] = pos+i;
        ret = po_write(fd,(byte*)big_buffer,nleft*4);
        if (ret != nleft*4)
        {
            printf("Write test 1: write returned %d instead of %d\n", ret, nleft);
            return(-1);
        }
        ret = po_lseek(fd, 0, PSEEK_SET);
        if (ret != 0)
        {
            printf("Write test 1: seek returned %d instead of %d\n", ret, 0);
            return(-1);
        }
        ret = po_read(fd,(byte*)big_buffer,(BIGBUFFINTS*sizeof(int)));
        if (ret != (BIGBUFFINTS*sizeof(int)))
        {
            printf("Write test 1: read returned %d instead of (BIGBUFFINTS*sizeof(int))\n", ret);
            return -1;
        }

        for(i=0;i<pos;i++)
            if (big_buffer[i] != 0)
            {
                printf("Write match 1 at %d got %d instead should be 0\n", i, big_buffer[i]);
                return -1;
            }

        for(i=pos;i<pos+nleft;i++)
        {
            if (big_buffer[i] != i)
            {
                printf("Write match 1A at %d got %d instead\n", i, big_buffer[i]);
                return -1;
            }
        }
        for(i=pos+nleft;i<BIGBUFFINTS;i++)
        {
            if (big_buffer[i] != 0)
            {
                printf("Write match 1A at %d got %d instead of zero\n", i, big_buffer[i]);
                return -1;
            }
        }
        if (nleft == nleft1)
        {
            nleft = nleft2;
            goto again;
        }
    }
    return 0;
}


int test_long_read(int fd)
{
int i, j, ret, pos, nleft, nleft2;

    ret = po_lseek(fd, 0, PSEEK_SET);
    if (ret != 0)
    {
        printf("Read test: seek returned %d instead of %d\n", ret, 0);
        return(-1);
    }
    for(i=0;i<BIGBUFFINTS;i++)
        big_buffer[i] = i;
    ret = po_write(fd,(byte*)big_buffer,(BIGBUFFINTS*sizeof(int)));
    if (ret != (BIGBUFFINTS*sizeof(int)))
    {
        printf("Write returned %d instead of (BIGBUFFINTS*sizeof(int))\n", ret);
        return -1;
    }
    for (j = 0; j < 10; j++)
    {
        pos = myrand()%BIGBUFFINTS;
        nleft2 = BIGBUFFINTS - pos;
        nleft = myrand()%nleft2;
        printf("Read Testing %d of 10000 POS %d, NLEFT %d NLEFT2 %d\n",j, pos, nleft, nleft2);
        ret = po_lseek(fd, pos*4, PSEEK_SET);
        if (ret != pos*4)
        {
            printf("Read test: seek returned %d instead of %d\n", ret, pos*4);
            return(-1);
        }
        for(i=0;i<nleft;i++)
            big_buffer[i] = 0;
        ret = po_read(fd,(byte*)big_buffer,nleft*4);
        if (ret != nleft*4)
        {
            printf("Read test: read returned %d instead of %d\n", ret, nleft);
            return(-1);
        }
        for(i=0;i<nleft;i++)
        {
            if (big_buffer[i] != (pos + i))
            {
                printf("Read match 1 at %d got %d instead\n", pos+i, big_buffer[i]);
                return -1;
            }
        }
        ret = po_lseek(fd, pos*4, PSEEK_SET);
        if (ret != pos*4)
        {
            printf("Read test 2: seek returned %d instead of %d\n", ret, pos*4);
            return(-1);
        }
        for(i=0;i<nleft2;i++)
            big_buffer[i] = 0;
        ret = po_read(fd,(byte*)big_buffer,nleft2*4);
        if (ret != nleft2*4)
        {
            printf("Read test 2 : read returned %d instead of %d\n", ret, nleft2);
            return(-1);
        }
        for(i=0;i<nleft2;i++)
        {
            if (big_buffer[i] != (pos + i))
            {
                printf("Read match 1 at %d got %d instead\n", pos+i, big_buffer[i]);
                return -1;
            }
        }
    }
    return 0;
}

void test_long_io()
{
    int i, ret;
    int fd;

    sprintf(filename[0], "\\bigfile.txt");

    printf("Creating %s\n", filename[0]);
    if ((fd = po_open(filename[0],(word)(PO_BINARY|PO_WRONLY|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
    {
         for(i=0;i<BIGBUFFINTS;i++)
             big_buffer[i] = i;
         ret = po_write(fd,(byte*)big_buffer,(BIGBUFFINTS*sizeof(int)));
         if (ret != (BIGBUFFINTS*sizeof(int)))
         {
            printf("Write returned %d instead of (BIGBUFFINTS*sizeof(int))\n", ret);
            return;
         }
         po_flush(fd);
         for (i = 0; i < 100; i++)
         {
            printf("Test outer loop at %d\n", i);
             if (test_long_write(fd) == -1)
                 break;
             if (test_long_read(fd) == -1)
                 break;

         }
    }
    else
    {
        printf("Open failed\n");
        return;
    }
    po_close(fd);
    pc_unlink(filename[0]);
}


void test_errors()
{
int i;
int fd1;


    for (i = 0; i < 1000; i++)
        pc_mkdir((byte *)"SUB_DIR");

    if ((fd1= po_open((byte *)"FILENAME",(word)(PO_BINARY|PO_WRONLY|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
    {
        po_close(fd1);
        for (i = 0; i < 1000; i++)
            po_open((byte *)"FILENAME",(word)(PO_BINARY|PO_WRONLY|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD));

    }
    for (i = 0; i < 1000; i++)
        pc_rmdir((byte *)"SUB_DIR");
    for (i = 0; i < 1000; i++)
        pc_unlink((byte *)"FILENAME");
    for (i = 0; i < 1000; i++)
        pc_mv((byte *)"NOTTHERE", (byte *)"STILLNOT");
}


#endif
