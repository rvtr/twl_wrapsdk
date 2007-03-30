#ifndef TWL_VLINK_H_
#define TWL_VLINK_H_

#ifndef _IOREAD
#define _IOREAD         0x01
#define _IOWRITE        0x02
#define _IORW           0x04
#define _IOEOF          0x08
#define _IOFLUSH        0x10
#define _IOERR          0x20
#define _IOSTRING       0x40
#endif

#ifdef __cplusplus
extern "C" {
#endif

void vlinkInit(void);

struct ffblk {
    char ff_reserved[23];
    char ff_attrib;
    unsigned short ff_ftime;
    unsigned short ff_fdate;
    struct {
        unsigned short low,high;
    } ff_fsize;
    char  ff_name[255+1];
};

typedef struct {
unsigned year;
unsigned mon;
unsigned day;
unsigned hour;
unsigned min;
unsigned sec;
unsigned msec;
} Vlink_dos_time;

int vlink_dos_put_console(char c);
int vlink_dos_putstring_console(char *str,int size);
int vlink_dos_stat_console(void);
int vlink_dos_get_console(void);
int vlink_dos_open(char *fname,int mode);
int vlink_dos_creat(char *fname,int mode);
int vlink_dos_read(int fd,char *buf,int size);
int vlink_dos_write(int fd,char *buf,int size);
int vlink_dos_close(int fd);
int vlink_dos_lseek(int fd,int ofs,int pos);
int vlink_dos_file_mode(char *fname,int mode,int action);
int vlink_dos_set_dta(struct ffblk *fbp);
int vlink_dos_find_first(char *fname,unsigned attrib);
int vlink_dos_find_next(void);
int vlink_dos_find_close(void);
int vlink_dos_get_ioctl(int fd);
int vlink_dos_get_file_time(int fd,unsigned short *timep);
int vlink_dos_get_time(Vlink_dos_time *dtp);
int vlink_dos_set_file_time(int fd,unsigned short *timep);
int vlink_dos_mkdir(char *path);
int vlink_dos_rmdir(char *path);
int vlink_dos_remove(char *fname);
int vlink_dos_rename(char *old,char *new);
int vlink_dos_getcwd(char *path,int drvno);
int vlink_dos_chdir(char *path);
int vlink_dos_getcdrv(void);
int vlink_dos_dup(int fd);
int vlink_dos_dup2(int oldfd,int newfd);
int vlink_dos_version(void);
int vlink_dos_exit(int retcode);
int vlink_dos_sleep(int ms);

int vlinkRead_VIO_pt(void *dramAddr, int nbytes);
int vlinkWrite_VIO_pt(void *dramAddr,int nbytes);

int i_vlink_init_osemu(void);

void vlink_dos_printf(const char *text, ...);
void vlink_dos_fprintf(int fd, const char *text, ...);


#ifdef __cplusplus
}
#endif

#endif // TWL_VLINK_H_
