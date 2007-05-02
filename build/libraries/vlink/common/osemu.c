/*
        OSEMU.C

    Copyright (c) 1995-2002 by Kyoto Micro Computer Co.,LTD.
    All Rights Reserved.

*/

#include <twl/os.h>
#include <twl/vlink.h>

void (*i_vlink_start_fnc)(void);
void (*i_vlink_end_fnc)(void);

#define VLINK_START     if(i_vlink_start_fnc!=0) i_vlink_start_fnc();
#define VLINK_END       if(i_vlink_end_fnc!=0) i_vlink_end_fnc();

#define __asm__         asm

#define PORTSIZE unsigned char

int vlink_dos_errno;

#define UBYTE unsigned char
#define UWORD unsigned short
#define ULONG unsigned

#define MONADR          __et2_vlink_monadr
#define MONADR_H        __et2_vlink_monadr_h
#define SHIFT_L         __et2_vlink_shift_l
#define SHIFT_R         __et2_vlink_shift_r
#define CMDWRS          __et2_vlink_cmdwrs
#define CMDRD           __et2_vlink_cmdrd
#define STAT            __et2_vlink_stat

ULONG MONADR=0;
ULONG SHIFT_L=0;
ULONG SHIFT_R=0;
ULONG MONADR_H=0;

volatile PORTSIZE *CMDWRS=0;
volatile PORTSIZE *CMDRD=0;
volatile PORTSIZE *STAT=0;


#define OSEMU           0x83
#define EMU_OPEN        0x0
#define EMU_CREAT       0x1
#define EMU_READ        0x2
#define EMU_WRITE       0x3
#define EMU_CLOSE       0x4
#define EMU_LSEEK       0x5
#define EMU_FILE_MODE   0x6

#define EMU_FIND_FIRST  0x8
#define EMU_FIND_NEXT   0x9
#define EMU_FIND_CLOSE  0xa
#define EMU_GET_IOCTL   0xb
#define EMU_GET_FILE_TM 0xc
#define EMU_GET_TIME    0xd
#define EMU_SET_FILE_TM 0xe
#define EMU_MKDIR       0xf
#define EMU_RMDIR       0x10
#define EMU_REMOVE      0x11
#define EMU_RENAME      0x12
#define EMU_GETCWD      0x13
#define EMU_CHDIR       0x14
#define EMU_GETCDRV     0x15


#define EMU_DUP         0x18
#define EMU_DUP2        0x19
#define EMU_VERSION     0x1a
#define EMU_EXIT        0x1b

#define EMU_SLEEP       0x1d
#define EMU_ARG_ENV     0x1e
/*
char *_dos_arg;
char *_dos_env;
*/

static struct ffblk *ffblkp;

static int putCMD_init(UBYTE c);
static int putCMD_not_use(UBYTE c);
static int not_use_vlink(void);

int (*putCMD)(UBYTE)            =putCMD_init;
void (*putPT)(UBYTE)            =(void (*)(UBYTE))not_use_vlink;
UBYTE (*getPT)(void)            =(UBYTE (*)(void))not_use_vlink;
UBYTE (*getPT_sync)(void)       =(UBYTE (*)(void))not_use_vlink;
void (*putPT_sync)(UBYTE)       =(void (*)(UBYTE))not_use_vlink;
void (*putPT_L)(ULONG)          =(void (*)(ULONG))not_use_vlink;
ULONG (*getPT_L)(void)          =(ULONG (*)(void))not_use_vlink;
void (*getPT_BLK)(UBYTE*,ULONG) =(void (*)(UBYTE*,ULONG))not_use_vlink;
void (*putPT_BLK)(UBYTE*,ULONG) =(void (*)(UBYTE*,ULONG))not_use_vlink;
UBYTE (*getSTAT)(void)          =(UBYTE (*)(void))not_use_vlink;

void (*putPT_flash)(void)       =(void (*)(void))not_use_vlink;


static unsigned vlink_sync_stat;
static unsigned vlink_not_use_osemu;

#include        <nitro/code32.h>

static asm int i_vlink_jtag_com_stat(void)     /* bit0==1 受信でデータあり / bit1==0 送信可能 */
{
        mrc     p14,0,r0,c0,c0, 0
        bx      lr
}

static asm void i_vlink_jtag_com_put(ULONG r0)
{
        mcr     p14,0,r0,c1,c0, 0
        bx      lr
}

static asm ULONG i_vlink_jtag_com_get(void)
{
        mrc     p14,0,r0,c1,c0, 0
        bx      lr
}

static asm int i_vlink_jtag_com_stat11(void)   /* bit30==1 受信でデータあり / bit29==0 送信可能 */
{
        mrc     p14,0,r0,c0,c1, 0
        bx      lr
}

static asm void i_vlink_jtag_com_put11(ULONG r0)
{
        mcr     p14,0,r0,c0,c5, 0
        bx      lr
}

static asm ULONG i_vlink_jtag_com_get11(void)
{
        mrc     p14,0,r0,c0,c5, 0
        bx      lr
}

static asm int i_vlink_jtag_cpuid(void)
{
#ifdef SDK_ARM9
        mrc     p15,0,r0,c0,c0, 0       /* bit[19..16] =7  ARMv6(ARM11) */
#else // SDK_ARM7
        mov     r0, #0x00050000
#endif
        bx      lr
}

static UBYTE VLC_getSTAT(void)
{
    return (i_vlink_jtag_com_stat() & 1);
}

static void VLC_putPT(UBYTE c)
{
    while((i_vlink_jtag_com_stat() & 2)!=0){
        ;
    }
    i_vlink_jtag_com_put((ULONG)c);
}

static int VLC_putCMD(UBYTE data)
{
    VLC_putPT(data);
    return 0;
}

static void VLC_putPT_L(ULONG c)
{
    while((i_vlink_jtag_com_stat() & 2)!=0){
        ;
    }
    i_vlink_jtag_com_put(c);
}


static UBYTE VLC_getPT(void)
{
    UBYTE c;
    while((i_vlink_jtag_com_stat() & 1)==0){
        ;
    }
    c=(i_vlink_jtag_com_get()>>24);
    return c;
}

static ULONG VLC_getPT_L(void)
{
    ULONG c;
    while((i_vlink_jtag_com_stat() & 1)==0){
        ;
    }
    c=i_vlink_jtag_com_get();
    return c;
}

static UBYTE VLC_getSTAT11(void)
{
    return (i_vlink_jtag_com_stat11() & 0x40000000);
}

static void VLC_putPT11(UBYTE c)
{
    while((i_vlink_jtag_com_stat11() & 0x20000000)!=0){
        ;
    }
    i_vlink_jtag_com_put11((ULONG)c);
}

static int VLC_putCMD11(UBYTE data)
{
    VLC_putPT11(data);
    return 0;
}

static void VLC_putPT_L11(ULONG c)
{
    while((i_vlink_jtag_com_stat11() & 0x20000000)!=0){
        ;
    }
    i_vlink_jtag_com_put11(c);
}


static UBYTE VLC_getPT11(void)
{
    UBYTE c;
    while((i_vlink_jtag_com_stat11() & 0x40000000)==0){
        ;
    }
    c=i_vlink_jtag_com_get11();
    return c;
}

static ULONG VLC_getPT_L11(void)
{
    ULONG c;
    while((i_vlink_jtag_com_stat11() & 0x40000000)==0){
        ;
    }
    c=i_vlink_jtag_com_get11();
    return c;
}

#define VLC_BLK         vlc_blk
#define EXT_CODE        0x12345678
#define EOF_CODE        0x87654321

static void set_longp(void *p, ULONG dt)
{
    switch((ULONG)p&3) {
    case 0:
            *((ULONG *)p)= dt;
        break;
    case 2:
        *((UWORD *)p)= dt;
        dt >>= 16;
        *((UWORD *)p+1)= dt;
            break;
    default:
        *(UBYTE *)p=dt;
        *((UBYTE *)p+1)=dt>>8;
        *((UBYTE *)p+2)=dt>>16;
        *((UBYTE *)p+3)=dt>>24;
    }
}

static void VLC_getPT_BLK(UBYTE *bufp,ULONG ct)
{
    ULONG data;
    while(ct>=4){
        set_longp(bufp,getPT_L());
        bufp += 4;
        ct -= 4;
    }
    if(ct!=0){
        data=getPT_L();
        do{
            *(bufp++)=(UBYTE)data;
            data >>= 8;
        }while(--ct) ;
    }
}


static ULONG get_longp(void *p)
{
    ULONG dt;

    switch((ULONG)p&3) {
    case 0:
            dt= *((ULONG *)p) ;
        break;
    case 2:
            dt=*((UWORD *)p+1);
        dt<<=16;
        dt|=*(UWORD *)(p);
        break;
    default:
        dt=*((UBYTE *)p+3);
        dt<<=8;
        dt|=*((UBYTE *)p+2);
        dt<<=8;
        dt|=*((UBYTE *)p+1);
        dt<<=8;
        dt|=*(UBYTE *)p;
    }
    return dt;
}

static void VLC_putPT_BLK(UBYTE *bufp,ULONG ct)
{
    ULONG data = 0;

    while(ct>=4){
        data = get_longp(bufp);
        putPT_L(data);
        bufp += 4;
        ct -= 4;
    }
    if(ct!=0){
        do{
            data <<= 8;
            --ct;
            data |= *(bufp+ct);
        }while(ct) ;
        putPT_L(data);
    }
}





#if KMC_BUF /* { */
static char *kmc_wp,*kmc_rp;
static char kmc_buffer[WDB_KMC_MTU];
#endif /* } */

void i_vlink_et2_vlink_tbl(void)
{
    __et2_vlink_monadr=0;
    __et2_vlink_monadr_h=0;
    __et2_vlink_shift_l=0;
    __et2_vlink_shift_r=0;
    __et2_vlink_cmdwrs=0;
    __et2_vlink_cmdrd=0;
    __et2_vlink_stat=0;
}

static void vlinkPutPTstring(char *p)
{
    char c;

    do{
        c=*(p++);
        putPT(c);
    } while (c!='\0') ;
}


#define putOSEMU(c) if(putCMD(c)) {VLINK_END;return -12345678;}

int vlink_dos_put_console(char c)
{
    VLINK_START
    putOSEMU(0);
    getPT_sync();
    putPT_BLK((UBYTE*)&c,1);
    putPT_flash();
    VLINK_END
    return 0;
}

int vlink_dos_putstring_console(char *str,int size)
{
    int len;
    int ct;
    int start;

    VLINK_START
    start=1;
    if(size==0){
        char *p;
        p=str;
        while(*p) ++p;
        size=p-str;
    }
    len=size;
    while(len){
        if(len>0x80){
            ct=0x80;
        }
        else{
            ct=len;
        }
        if(start){
            putOSEMU(ct-1);
            start=0;
        }
        else{
            putPT(ct-1);
        }
        getPT_sync();
        putPT_BLK((UBYTE *)str,ct);
        len -= ct;
        str += ct;
    }
    putPT_flash();
    VLINK_END
    return size;
}


int vlink_dos_stat_console(void)
{
    char c;

    VLINK_START
    putOSEMU(0x80);
    c=getPT();
    VLINK_END
    if(c=='\x1b'){
        int i;
        for(i=0;i<1000000;++i) ;
        c=0;
    }
    return c;
}

int vlink_dos_get_console(void)
{
    char c;

    VLINK_START
    while((c=vlink_dos_stat_console())=='\0') ;
    VLINK_END
    return c;
}


int vlink_dos_open(char *fname,int mode)
{
    int fd;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_OPEN);
    vlinkPutPTstring(fname);
    putPT(mode);
    fd=getPT();
    if(fd==0xff){
        vlink_dos_errno=getPT();
        fd=-1;
    }
    VLINK_END
    return fd;
}

int vlink_dos_creat(char *fname,int mode)
{
    int fd;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_CREAT);
    vlinkPutPTstring(fname);
    putPT(mode);
    fd=getPT();
    if(fd==0xff){
        vlink_dos_errno=getPT();
        fd=-1;
    }
    VLINK_END
    return fd;
}

int vlink_dos_read(int fd,char *buf,int size)
{
    int rval;

    if(fd==0){        /* CON IN ? */
        if(size<0) return 0;
        VLINK_START
        *buf = vlink_dos_get_console();
        vlink_dos_write(1,buf,1);
        if(*buf=='\r'){
//            *buf = 0;        /* END */
            vlink_dos_write(1,"\n",1);
        }
        VLINK_END
        return 1;
    }
    if(size<0) return -1;
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_READ);
    putPT(fd);
    putPT_L(size);
    getPT_BLK((UBYTE *)buf,size);
    putPT_sync(0);
    rval=getPT_L();
    if(rval==-1){
        vlink_dos_errno=getPT();
    }
    VLINK_END
    return rval;
}

int vlink_dos_write(int fd,char *buf,int size)
{
    int rval;

    if(size<0) return -1;
    if(fd==1 || fd==2){        /* CON OUT ? */
        return vlink_dos_putstring_console(buf,size);
    }
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_WRITE);
    putPT(fd);
    putPT_L(size);
    putPT_BLK((UBYTE *)buf,size);
    rval=getPT_L();
    if(rval==-1){
        vlink_dos_errno=getPT();
    }
    VLINK_END
    return rval;
}

int vlink_dos_close(int fd)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_CLOSE);
    putPT(fd);
    rval=0;
    if(getPT()!=0){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_lseek(int fd,int ofs,int pos)
{
    int ret_pos;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_LSEEK);
    putPT(fd);
    putPT_L(ofs);
    putPT(pos);
    if((ret_pos=getPT_L())==-1){
        vlink_dos_errno=getPT();
    }
    VLINK_END
    return ret_pos;
}

int vlink_dos_file_mode(char *fname,int mode,int action)
{
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_FILE_MODE);
    vlinkPutPTstring(fname);
    putPT_L(mode);
    putPT(action);
    if((mode=getPT_L())==-1){
        vlink_dos_errno=getPT();
    }
    VLINK_END
    return mode;
}

int vlink_dos_set_dta(struct ffblk *fbp)
{
    ffblkp=fbp;
    return 0;
}


int vlink_dos_find_first(char *fname,unsigned attrib)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_FIND_FIRST);
    vlinkPutPTstring(fname);
    rval=0;
    if(getPT()!=0){
        rval=-1;
        goto ret;
    }
    getPT_BLK((UBYTE *)ffblkp,sizeof(struct ffblk));
ret:
    VLINK_END
    return rval;
}

int vlink_dos_find_next(void)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_FIND_NEXT);
    rval=0;
    if(getPT()!=0){
        rval=-1;
        goto ret;
    }
    getPT_BLK((UBYTE *)ffblkp,sizeof(struct ffblk));
ret:
    VLINK_END
    return rval;
}

int vlink_dos_find_close(void)
{
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_FIND_CLOSE);
    putPT_flash();
    VLINK_END
    return 0;
}

int vlink_dos_get_ioctl(int fd)
{
    int rval;

    if(fd==1){        /* CON OUT ? */
        return 0x80;
    }
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_GET_IOCTL);
    putPT(fd);
    if((rval=getPT())==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_get_file_time(int fd,unsigned short *timep)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_GET_FILE_TM);
    putPT(fd);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    else{
        *timep=(unsigned short)getPT_L();
        *(timep+1)=(unsigned short)getPT_L();
    }
    VLINK_END
    return rval;
}

int vlink_dos_get_time(Vlink_dos_time *dtp)
{
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_GET_TIME);
    getPT_BLK((UBYTE *)dtp,sizeof(Vlink_dos_time));
    VLINK_END
    return 0;
}


int vlink_dos_set_file_time(int fd,unsigned short *timep)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_SET_FILE_TM);
    putPT(fd);
    putPT_L(*timep);
    putPT_L(*(timep+1));
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_mkdir(char *path)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_MKDIR);
    vlinkPutPTstring(path);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_rmdir(char *path)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_RMDIR);
    vlinkPutPTstring(path);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_remove(char *fname)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_REMOVE);
    vlinkPutPTstring(fname);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_rename(char *old,char *new)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_RENAME);
    vlinkPutPTstring(old);
    vlinkPutPTstring(new);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}


int vlink_dos_getcwd(char *path,int drvno)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_GETCWD);
    putPT(drvno);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    else{
        do{
            *path=getPT();
        }while(*(path++)!='\0') ;
    }
    VLINK_END
    return rval;
}

int vlink_dos_chdir(char *path)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_CHDIR);
    vlinkPutPTstring(path);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_getcdrv(void)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_GETCDRV);
    rval=getPT();
    VLINK_END
    return rval;
}

int vlink_dos_dup(int fd)
{
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_DUP);
    putPT(fd);
    if((fd=getPT())==0xff){
        vlink_dos_errno=getPT();
        fd=-1;
    }
    VLINK_END
    return fd;
}

int vlink_dos_dup2(int oldfd,int newfd)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_DUP2);
    putPT(oldfd);
    putPT(newfd);
    rval=0;
    if(getPT()==0xff){
        vlink_dos_errno=getPT();
        rval=-1;
    }
    VLINK_END
    return rval;
}

int vlink_dos_version(void)
{
    int rval;

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_VERSION);
    rval=getPT_L();
    VLINK_END
    return rval;
}

int vlink_dos_exit(int retcode)
{
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_EXIT);
    putPT_L(retcode);
    putPT_flash();
    VLINK_END
    for(;;) ;
}

int vlink_dos_sleep(int ms)
{
    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_SLEEP);
    putPT_L(ms);
    getPT();
    VLINK_END
    return 0;
}
/*
int vlink_dos_arg_env(void)
{
    int arg_size;
    int env_size;
    char *malloc();

    VLINK_START
    putOSEMU(OSEMU);
    putPT(EMU_ARG_ENV);
    arg_size=getPT_L();
    env_size=getPT_L();
    vlink_dos_arg=malloc(arg_size+1);
    vlink_dos_env=malloc(env_size+1);
    getPT_BLK((UBYTE *)_dos_arg,arg_size);
    putPT_sync(0);
    getPT_BLK((UBYTE *)_dos_env,env_size);
    *(_dos_arg+arg_size)='\0';
    *(_dos_env+env_size)='\0';
    VLINK_END
    return 0;
}
*/


#define READ_VIO        0
#define WRITE_VIO        1

int vlinkRead_VIO_pt(void *dramAddr, int nbytes)
{
    int size;

    if(nbytes<=0) return nbytes;
    VLINK_START
    if(vlink_sync_stat==1){
        size=getPT_L();
        if(size>nbytes){
            nbytes=0;
            goto ret;
        }
        nbytes=size;
    }
    else{
        putPT(0x82);
        putPT(READ_VIO);
        putPT_L(nbytes);
    }
    getPT_BLK((UBYTE *)dramAddr,nbytes);
ret:
    VLINK_END
    return nbytes;
}

int vlinkWrite_VIO_pt(void *dramAddr,int nbytes)
{
    if(nbytes<=0) return nbytes;
    VLINK_START
    if(vlink_sync_stat!=1){
        putPT(0x82);
        putPT(WRITE_VIO);
    }
    putPT_L(nbytes);
    if(vlink_sync_stat==2){
        getPT();
    }

    putPT_BLK((UBYTE *)dramAddr,nbytes);
    putPT_flash();
    VLINK_END
    return nbytes;
}

int i_vlink_init_osemu(void)
{
    int rval;
    //ULONG *vtbl;
    //char *malloc();

//    vlinkInit();    // add by yutaka

    VLINK_START
    rval=0;

    if((i_vlink_jtag_cpuid() & 0x000f0000)>=0x00070000){        /* ARM11 */
       putCMD=vlink_not_use_osemu ? putCMD_not_use:VLC_putCMD11;
       putPT=VLC_putPT11;
       getPT=VLC_getPT11;
       putPT_L=VLC_putPT_L11;
       getPT_L=VLC_getPT_L11;
       getSTAT=VLC_getSTAT11;
    }
    else {
        putCMD=vlink_not_use_osemu ? putCMD_not_use:VLC_putCMD;
        putPT=VLC_putPT;
        getPT=VLC_getPT;
        putPT_L=VLC_putPT_L;
        getPT_L=VLC_getPT_L;
        getSTAT=VLC_getSTAT;
    }
    getPT_BLK=VLC_getPT_BLK;
    putPT_BLK=VLC_putPT_BLK;
    if(getSTAT()){
        getPT();
    }
    //arm_loop_max=1000;
    rval=2;

    VLINK_END
    return rval;
}

static int putCMD_init(UBYTE c)
{
    i_vlink_init_osemu();
    return putCMD(c);
}

static int putCMD_not_use(UBYTE c)
{
    return 1;
}

static int not_use_vlink(void)
{
    return 0;
}
