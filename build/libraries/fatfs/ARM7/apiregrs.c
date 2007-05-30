/*****************************************************************************
    apiregrs.c   - RTFS Excerciser

     This program performs two functions. It calls virtually all of the API
     routines plus it stress tests the system for driver bugs and memory leaks.
     It works by repeatedly opening a disk and then entering an inner loop
     which creates a directory and then creates N subdirectories below that.
     Finally the inner loop creates NUSERFILES files, writes to them, reads from
     them, seeks, truncates, closes, renames and deletes them. Along the way
     it check set current working directory and get working directory. Finally
     the inner loop deletes all of the subdirectories it created and compares
     the current disk free space to the free space before it started. These
     should be the same. After the inner loop completes the outer loop closes
     the drive and then reopens it to continue the test.

     There are a few functions that do not get tested, they are:
        pc_gfirst
        pc_gnext
        pc_gdone
        pc_abort

     Not all modes of po_open and po_lseek are tested either. Nor does it
     test your port in multitasking mode. You may modify this program and
     run it in multiple threads if you wish.

     The following parameters may be changed:

        USEPRINTF -  Set this to zero to run completely quietly. If this
                     is done you should set a break point in regress_error()
                     to catch errors.
        test_drive[] - The drive where the test will occur.
        test_dir[]   - The directory where the test will occur
        INNERLOOP   - The Number of times we run the inner loop
        OUTERLOOP   - The Number of times we run the inner loop
        SUBDIRDEPTH  - The depth of the tested subdirectories.
        NSUBDIRS     - The number of subdirectories below test_dir. Must
                       be less then 26. Each of these directories will
                       have SUBDIRDEPTH subdirectories below it.


To run the program type REGRESS.

*****************************************************************************/

#include <rtfs.h>

#define RGE_FLTDRIVE       3
#define RGE_FREEERROR      4
#define RGE_LEAKERROR      5
#define RGE_MKDIR          6
#define RGE_SCWD           7
#define RGE_MKDIRERR       9
#define RGE_PWD           10
#define RGE_RMDIR         11
#define RGE_DSKCLOSE      12
#define RGE_OPEN          13
#define RGE_SEEK          14
#define RGE_WRITE         15
#define RGE_READ          16
#define RGE_TRUNC         17
#define RGE_FLUSH         18
#define RGE_CLOSE         19
#define RGE_UNLINK        20
#define RGE_MV            21
#define RGE_CHSIZE        22
#define RGE_DELTREE       23
#define RGE_ERRNO         24
#define RGE_LONGFILETEST  25
#define RGE_FILETEST      26
#define RGE_LARGEFILETEST 27


/* Porting issues */
#define VERBOSE   1             /* Set to zero for quiet operation */
byte    test_drive[8];         /* The drive where the test will occur */
#define NLONGS 512                /* Longs to write in file write test */
#define SUBDIRDEPTH  10            /* Depth of subdirectories */
#define NSUBDIRS     8            /* <= 26 Number of subdirs at below RTFSTEST */



dword test_rtfs_buf[NLONGS];      /* Used in the file write test */

#if (VFAT)  /* Small piece of compile time VFAT vs's NONVFAT code  */
#if (INCLUDE_CS_UNICODE)
/* Make names a little shorter in Unicode, otherwise path's get too long */
byte _test_dir[] = CS_OP_ASCII("Directory");       /* Test will occur in this Directory */
byte _test_file_name[] = CS_OP_ASCII("Long File Name");
byte _test_newfile_name[] =CS_OP_ASCII("New Long File Name");
byte _test_subdir_name[] = CS_OP_ASCII("Subdir");
#else
byte _test_dir[] = CS_OP_ASCII("RTFS_Test_Directory");       /* Test will occur in this Directory */
byte _test_file_name[] = CS_OP_ASCII("Long File Name");
byte _test_newfile_name[] =CS_OP_ASCII("New Long File Name");
byte _test_subdir_name[] = CS_OP_ASCII("Subdir");
#endif
#else
byte _test_dir[] = CS_OP_ASCII("RTFSTEST");      /* Test will occur in this Directory */
byte _test_file_name[] = CS_OP_ASCII("FILE");
byte _test_newfile_name[] =CS_OP_ASCII("NEWFILE");
byte _test_subdir_name[] = CS_OP_ASCII("SUBDIR");
#endif
/* Above strings are copied into the following buffers in native cha set */
byte test_dir[40];
byte test_file_name[40];
byte test_newfile_name[40];
byte test_subdir_name[40];

#define INNERLOOP   4              /* Number of times we run the tes suite
                                      between open and close. */
#define OUTERLOOP   20              /* Number of times we open the drive
                                      run the inner loop and close the drive */

BOOLEAN do_test(int loop_count, BOOLEAN do_clean);
BOOLEAN do_file_test(int loop_count, BOOLEAN do_clean);
#if (VFAT)
BOOLEAN do_long_file_test(BOOLEAN do_clean);
#endif

BOOLEAN do_rm(byte *buffer, int level);
void regress_error(int error);
BOOLEAN check_errno(int expected_error);
BOOLEAN do_buffered_file_test(BOOLEAN do_clean);

/* Copy strings to native character sets */
void setup_regress_strings()
{
    /* Set up strings in native character set */
    CS_OP_ASCII_TO_CS_STR(test_dir, (byte *) _test_dir);
    CS_OP_ASCII_TO_CS_STR(test_file_name, (byte *) _test_file_name);
    CS_OP_ASCII_TO_CS_STR(test_newfile_name, (byte *) _test_newfile_name);
    CS_OP_ASCII_TO_CS_STR(test_subdir_name, (byte *) _test_subdir_name);
}


#define PATHSIZE 256

BOOLEAN pc_regression_test(byte *driveid, BOOLEAN do_clean) /* __api__ */
{
int inner_loop;
int outer_loop;
long nfree, nfree_too;
dword blocks_total, blocks_free, blocks_free_too;

    rtfs_cs_strcpy(test_drive, driveid);

    setup_regress_strings();

    for (outer_loop = 0; outer_loop < OUTERLOOP; outer_loop++)
    {
#if (VERBOSE)
    RTFS_PRINT_LONG_1((dword) outer_loop, PRFLG_NL);
#endif

        if (!pc_set_default_drive(test_drive))
            { regress_error(RGE_FLTDRIVE); return(FALSE);}

        /* If test_dir is not there, deltree will just fail */
        pc_deltree(test_dir);
        /* Test buffered IO support. Loop a few times to check for leaks */
        for (inner_loop = 0; inner_loop < 4; inner_loop++)
        {
            RTFS_PRINT_STRING_1(USTRING_RTFSDEM_16, PRFLG_NL); /* "Performing buffered file io test" */
            if (!do_buffered_file_test(do_clean))
                return(FALSE);
        }
        for (inner_loop = 0; inner_loop < INNERLOOP; inner_loop++)
        {
#if USEPRINTF
#if (!VERBOSE)
            RTFS_PRINT_STRING_1(USTRING_RTFSDEM_01,PRFLG_NL); /* "+" */
#endif
#endif
            /* Check freespace */
            nfree = pc_free(test_drive, &blocks_total, &blocks_free);
            if (!nfree)
            {
                regress_error(RGE_FREEERROR);
                return(FALSE);
            }
            if (!do_test(inner_loop, do_clean))  /* Call the main test routine */
                return(FALSE);
            /* Check freespace again. They should match */

            if (!do_clean)   /* If not cleaning up don't recheck freespace */
                return(TRUE);/* And return */
            nfree_too = pc_free(test_drive, &blocks_total, &blocks_free_too);
            if (!nfree_too)
            {
                regress_error(RGE_FREEERROR);
                return(FALSE);
            }
            if (blocks_free_too != blocks_free)
            {
                regress_error(RGE_LEAKERROR);
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

/*
    Make the test directory
    loop
        Step into the test directory
        Make another subdiretory
            loop
                Make N deep subdirectories
                    Change into each
                    compare what we know is the directory with
                    what pc_pwd returns.
            End loop
            In the lowest level directory
                loop
                    create a file
                    open it with multiple file descriptors
                    loop
                        write to it on multiple FDs
                    loop
                        seek and read  on multiple FDs. Testing values
                    flush
                    truncate
                    close
                end loop
                loop
                    rename
                    delete
                end loop
            now delete all of the subdirectories

*/
void build_subdir_name(byte *p, int index)
{
byte c;
/* Create SubDirA, or SubDirB.. SubDir('A'+index) */
    rtfs_cs_strcpy(p,test_subdir_name);
    p = CS_OP_GOTO_EOS(p);
    /* Put A, or B, C, D.. at the end of the string */
    c = CS_OP_ASCII('A');
    c = (byte) (c + index);
    CS_OP_ASSIGN_ASCII(p,c);
    CS_OP_INC_PTR(p);
    CS_OP_TERM_STRING(p);
}

BOOLEAN do_test(int loop_count, BOOLEAN do_clean)
{
    int i;
    int j;
    byte *buffer;
    byte *buffer3;
    byte *buffer4;
    byte *home;
    byte *p;
    BLKBUFF *scratch_buffer;
    BLKBUFF *scratch_buffer3;
    BLKBUFF *scratch_buffer4;
    BLKBUFF *scratch_home;

    scratch_buffer = pc_scratch_blk();
    scratch_buffer3 = pc_scratch_blk();
    scratch_buffer4 = pc_scratch_blk();
    scratch_home = pc_scratch_blk();

    if (!(scratch_buffer && scratch_buffer3 && scratch_buffer4 && scratch_home))
    {
        if (scratch_buffer)
            pc_free_scratch_blk(scratch_buffer);
        if (scratch_buffer3)
            pc_free_scratch_blk(scratch_buffer3);
        if (scratch_buffer4)
            pc_free_scratch_blk(scratch_buffer4);
        if (scratch_home)
            pc_free_scratch_blk(scratch_home);
        return(FALSE);
    }
    buffer = (byte *)scratch_buffer->data;
    buffer3 = (byte *)scratch_buffer3->data;
    buffer4 = (byte *)scratch_buffer4->data;
    home = (byte *)scratch_home->data;

#if (VERBOSE)
    RTFS_PRINT_STRING_2(USTRING_RTFSDEM_08, test_dir,PRFLG_NL); /* "Creating Subdirectory:" */
#endif
    /* Delete the test dir if it exists */
    if (pc_isdir(test_dir))
    {
        if (!pc_deltree(test_dir))
        { regress_error(RGE_DELTREE); goto return_false;}
    }
    else
       { if (!check_errno(PENOENT)) goto return_false;}
    /* Create the test dir if it exists */
    if (!pc_mkdir(test_dir))
        { regress_error(RGE_MKDIR); goto return_false;}
    if (!check_errno(0)) goto return_false;
    if (!pc_set_cwd(test_dir))
        {regress_error(RGE_SCWD); goto return_false;}
    if (!check_errno(0)) goto return_false;

    /* Make the top level subdirs and subdirs down to layer SUBDIRDEPTH */
    /* Make D:\RTFS_Test_Directory string in native char set */
/*  rtfs_cs_strcpy(home, test_drive); */
    p = home;
/*  p = CS_OP_GOTO_EOS(p); */
    CS_OP_ASSIGN_ASCII(p, '\\');
    CS_OP_INC_PTR(p);
    CS_OP_TERM_STRING(p);
    rtfs_cs_strcat(home, test_dir);

    for (i = 0; i < NSUBDIRS; i++)
    {
        if (!pc_set_cwd(home))
            { regress_error(RGE_SCWD); goto return_false;}
        if (!check_errno(0)) goto return_false;
        /* Make the top level subdirs */
        build_subdir_name(buffer, i);
#if (VERBOSE)
        RTFS_PRINT_STRING_2(USTRING_RTFSDEM_09, buffer,PRFLG_NL); /* "Creating Subdirectory  " */
#endif

        if (!pc_mkdir(buffer))
            { regress_error(RGE_MKDIR); goto return_false;}
        if (!check_errno(0)) goto return_false;
        for (j = 0; j < SUBDIRDEPTH; j++)
        {
            if (!pc_set_cwd(home))
                { regress_error(RGE_SCWD); goto return_false;}
            if (!check_errno(0)) goto return_false;

            /* Now make SubdirA\\Subdir in native char set */
            p = buffer;
            p = CS_OP_GOTO_EOS(p);
            CS_OP_ASSIGN_ASCII(p,'\\');
            CS_OP_INC_PTR(p);
            CS_OP_TERM_STRING(p);
            rtfs_cs_strcat(buffer,test_subdir_name);
#if (VERBOSE)
            RTFS_PRINT_STRING_2(USTRING_RTFSDEM_10, buffer,PRFLG_NL); /* "Creating Subdirectory   " */
#endif

            /* Save D:\RTFS_Test_Directory\SubdirA\Subdir in native char set */
            /* For later comparison with get_pwd results */
            rtfs_cs_strcpy(buffer4, home);
            p = buffer4;
            p = CS_OP_GOTO_EOS(p);
            CS_OP_ASSIGN_ASCII(p,'\\');
            CS_OP_INC_PTR(p);
            CS_OP_TERM_STRING(p);
            rtfs_cs_strcat(buffer4, buffer);
            /* Create SubdirA\\Subdir under the test directory  */
            if (!pc_mkdir(buffer))
                { regress_error(RGE_MKDIR); goto return_false;}
            if (!check_errno(0)) goto return_false;
            /* Create a dir. We know this will fail. Force error recovery */
            if (pc_mkdir(buffer))
                { regress_error(RGE_MKDIRERR); goto return_false;}
            if (!check_errno(PEEXIST)) goto return_false;
            /* Go into the new directory */
            if (!pc_set_cwd(buffer))
                { regress_error(RGE_SCWD); goto return_false;}
            if (!check_errno(0)) goto return_false;
            /* Get the dir string */
            /* Should be:D:\RTFS_Test_Directory\SubdirA\\Subdir */
            if (!pc_pwd(test_drive, buffer3))
                { regress_error(RGE_PWD); goto return_false;}
            if (!check_errno(0)) goto return_false;
            /* Compare with saved vesrion */
            if (rtfs_cs_strcmp(buffer4, buffer3) != 0)
                { regress_error(RGE_PWD); goto return_false;}
        }
    }

#if (VFAT)
    /* Do the long file test */
    RTFS_PRINT_STRING_1(USTRING_RTFSDEM_15, PRFLG_NL); /* "Performing long file name test" */
    if (!do_long_file_test(do_clean))
    {
        regress_error(RGE_LONGFILETEST);
        goto return_false;
    }
#endif
    /* Do the file test */
    if (!do_file_test(loop_count, do_clean))
    {
        regress_error(RGE_FILETEST);
        goto return_false;
    }
    /* DELETE the  subdirs */
    if (!pc_set_cwd(home))
        { regress_error(RGE_SCWD); goto return_false;}
    if (!check_errno(0)) goto return_false;
    if (do_clean && (loop_count & 1))
    {
        /* Manually remove subdirs on odd loops */
        for (i = 0; i < NSUBDIRS; i++)
        {
            /* Delete sub directories SubdirectoryA\SUBDIR\SUBDIR ... */
            for (j = SUBDIRDEPTH; j > 0; j--)
            {
                build_subdir_name(buffer, i);
                if (!do_rm(buffer, j))
                    goto return_false;
            }
            /* Delete sub directories SUB_? */
            build_subdir_name(buffer, i);
            if (!pc_rmdir(buffer))
                { regress_error(RGE_RMDIR); goto return_false;}
            if (!check_errno(0)) goto return_false;
        }
    }
    /* Delete the test dir */
    /* Now make .. in native char set */
    p = buffer;
    CS_OP_ASSIGN_ASCII(p,'.');
    CS_OP_INC_PTR(p);
    CS_OP_ASSIGN_ASCII(p,'.');
    CS_OP_INC_PTR(p);
    CS_OP_TERM_STRING(p);
    if (!pc_set_cwd(buffer))
        { regress_error(RGE_SCWD); goto return_false;}
    if (!check_errno(0)) goto return_false;
    if (do_clean && (loop_count & 1))
    {
        if (!pc_rmdir(test_dir))
            { regress_error(RGE_RMDIR); goto return_false;}
        if (!check_errno(0)) goto return_false;
    }
    else if (do_clean)
    {
        if (!pc_deltree(test_dir))
            { regress_error(RGE_DELTREE); goto return_false;}
        if (!check_errno(0)) goto return_false;
    }
    pc_free_scratch_blk(scratch_buffer);
    pc_free_scratch_blk(scratch_buffer3);
    pc_free_scratch_blk(scratch_buffer4);
    pc_free_scratch_blk(scratch_home);
    return(TRUE);
return_false:
    pc_free_scratch_blk(scratch_buffer);
    pc_free_scratch_blk(scratch_buffer3);
    pc_free_scratch_blk(scratch_buffer4);
    pc_free_scratch_blk(scratch_home);
    return(FALSE);
}

/* Delete a subdir at level */
BOOLEAN do_rm(byte *buffer, int level)                              /*__fn__*/
{
int i;
byte *p;
    for (i = 0; i < level; i++)
    {
        p = buffer;
        p = CS_OP_GOTO_EOS(p);
        CS_OP_ASSIGN_ASCII(p,'\\');
        CS_OP_INC_PTR(p);
        CS_OP_TERM_STRING(p);
        rtfs_cs_strcat(buffer,test_subdir_name);
    }
#if (VERBOSE)
    RTFS_PRINT_STRING_2(USTRING_RTFSDEM_11, buffer,PRFLG_NL); /* "Removing Directory  " */
#endif
    if (!pc_rmdir(buffer))
        { regress_error(RGE_RMDIR); return(FALSE);}
    if (!check_errno(0)) return(FALSE);
    return(TRUE);
}

#if (VFAT)
byte *plong_name;
void create_long_name(int len, byte c);
int write_long_name(int len, byte c);
int reopen_long_name(int len, byte c);
int remove_long_name(int len, byte c);

BOOLEAN do_long_file_test(BOOLEAN do_clean)
{
   /* Use this buffer since under unicode we need more than 512 bytes to test */
    plong_name = (byte *) &test_rtfs_buf[0];
    if (write_long_name(32, (byte) 'X')) goto big_error;
    if (write_long_name(64, (byte) 'X')) goto big_error;
    if (write_long_name(63, (byte) 'X')) goto big_error;
    if (write_long_name(127,(byte) 'X')) goto big_error;
    if (write_long_name(34, (byte) 'X')) goto big_error;
    if (write_long_name(66, (byte) 'X')) goto big_error;
    if (write_long_name(68, (byte) 'X')) goto big_error;
    if (write_long_name(124,(byte) 'X')) goto big_error;
    if (write_long_name(221,(byte) 'X')) goto big_error;
    if (write_long_name(255,(byte) 'X')) goto big_error;
    /* This one should fail */
    if (!write_long_name(256,(byte) 'X')) goto big_error;
    if (write_long_name(66, (byte) 'Y')) goto big_error;
    if (write_long_name(68, (byte) 'Y')) goto big_error;
    if (write_long_name(124,(byte) 'Y')) goto big_error;
    if (write_long_name(221,(byte) 'Y')) goto big_error;
    if (write_long_name(255,(byte) 'Y')) goto big_error;
    if (write_long_name(32, (byte) 'Y')) goto big_error;
    if (write_long_name(64, (byte) 'Y')) goto big_error;
    if (write_long_name(63, (byte) 'Y')) goto big_error;
    if (write_long_name(127,(byte) 'Y')) goto big_error;
    if (write_long_name(34, (byte) 'Y')) goto big_error;
    if (reopen_long_name(32, (byte) 'X')) goto big_error;
    if (reopen_long_name(64, (byte) 'X')) goto big_error;
    if (reopen_long_name(63, (byte) 'X')) goto big_error;
    if (reopen_long_name(127,(byte) 'X')) goto big_error;
    if (reopen_long_name(34, (byte) 'X')) goto big_error;
    if (reopen_long_name(66, (byte) 'X')) goto big_error;
    if (reopen_long_name(68, (byte) 'X')) goto big_error;
    if (reopen_long_name(124,(byte) 'X')) goto big_error;
    if (reopen_long_name(221,(byte) 'X')) goto big_error;
    if (reopen_long_name(255,(byte) 'X')) goto big_error;
    if (reopen_long_name(66, (byte) 'Y')) goto big_error;
    if (reopen_long_name(68, (byte) 'Y')) goto big_error;
    if (reopen_long_name(124,(byte) 'Y')) goto big_error;
    if (reopen_long_name(221,(byte) 'Y')) goto big_error;
    if (reopen_long_name(255,(byte) 'Y')) goto big_error;
    if (reopen_long_name(32, (byte) 'Y')) goto big_error;
    if (reopen_long_name(64, (byte) 'Y')) goto big_error;
    if (reopen_long_name(63, (byte) 'Y')) goto big_error;
    if (reopen_long_name(127,(byte) 'Y')) goto big_error;
    if (reopen_long_name(34, (byte) 'Y')) goto big_error;
/* Return here if you want to see the results */
/*    pc_free_scratch_blk(scratch); */
/* return(1);  */
    if (do_clean)
    {
    if (remove_long_name(32, (byte) 'X')) goto big_error;
    if (remove_long_name(64, (byte) 'X')) goto big_error;
    if (remove_long_name(63, (byte) 'X')) goto big_error;
    if (remove_long_name(127,(byte) 'X')) goto big_error;
    if (remove_long_name(34, (byte) 'X')) goto big_error;
    if (remove_long_name(66, (byte) 'X')) goto big_error;
    if (remove_long_name(68, (byte) 'X')) goto big_error;
    if (remove_long_name(124,(byte) 'X')) goto big_error;
    if (remove_long_name(221,(byte) 'X')) goto big_error;
    if (remove_long_name(255,(byte) 'X')) goto big_error;
    if (remove_long_name(66, (byte) 'Y')) goto big_error;
    if (remove_long_name(68, (byte) 'Y')) goto big_error;
    if (remove_long_name(124,(byte) 'Y')) goto big_error;
    if (remove_long_name(221,(byte) 'Y')) goto big_error;
    if (remove_long_name(255,(byte) 'Y')) goto big_error;
    if (remove_long_name(32, (byte) 'Y')) goto big_error;
    if (remove_long_name(64, (byte) 'Y')) goto big_error;
    if (remove_long_name(63, (byte) 'Y')) goto big_error;
    if (remove_long_name(127,(byte) 'Y')) goto big_error;
    if (remove_long_name(34, (byte) 'Y')) goto big_error;
    }
    return(TRUE);
big_error:
    return(FALSE);
}

void create_long_name(int len, byte c)
{
    int i;
    byte *p;

    p = plong_name;
    for (i = 0; i < len; i++)
    {
        CS_OP_ASSIGN_ASCII(p,c);
        CS_OP_INC_PTR(p);
    };
    CS_OP_TERM_STRING(p);
}

int write_long_name(int len, byte c)
{
    int fd;
    create_long_name(len, c);

    if ((fd = po_open(plong_name,(word)(PO_BINARY|PO_WRONLY|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
    {
       po_close(fd);
       return(0);
    }
    else
    {
       return(-1);
    }

}
int reopen_long_name(int len, byte c)
{
    int fd;
    create_long_name(len, c);
    if ((fd = po_open(plong_name,(word)(PO_BINARY|PO_WRONLY),(word)(PS_IWRITE | PS_IREAD))) >= 0)
    {
       po_close(fd);
       return(0);
    }
    else
    {
       return(-1);
    }
}
int remove_long_name(int len, byte c)
{
    create_long_name(len, c);
    if (!pc_unlink(plong_name))
    {
       return(-1);
    }
    else
       return(0);
}


#endif


BOOLEAN do_buffered_file_test(BOOLEAN do_clean)
{
PCFD fd1, fd2;
word i,j, *wbuff;
    wbuff = (word *) &test_rtfs_buf[0];

    /* Write 1024 bytes, close to flush, re-open and read */
    fd1 = po_open(test_file_name, PO_RDWR|PO_CREAT|PO_TRUNC|PO_BUFFERED, PS_IWRITE|PS_IREAD);
    if (fd1 < 0)
        { regress_error(RGE_OPEN); return(FALSE);}
    for (i = 0; i < 512; i++)
        if (po_write(fd1, (byte *)&i, 2) != 2)
            { regress_error(RGE_WRITE); return(FALSE);}
    po_close(fd1);
    fd1 = po_open(test_file_name, PO_RDWR|PO_BUFFERED, 0);
    if (fd1 < 0)
        { regress_error(RGE_OPEN); return(FALSE);}
    for (i = 0; i < 512; i++)
        if ( (po_read(fd1, (byte *)&j, 2) != 2) || j != i)
            { po_close(fd1); regress_error(RGE_READ); return(FALSE);}
    po_close(fd1);
    /* Read two buffered viewports of the same file.
       Note: Buffer thrashing occurs here */
    fd1 = po_open(test_file_name, PO_RDWR|PO_BUFFERED, PS_IWRITE|PS_IREAD);
    if (fd1 < 0)
        { regress_error(RGE_OPEN); return(FALSE);}
    fd2 = po_open(test_file_name, PO_RDWR|PO_BUFFERED, PS_IWRITE|PS_IREAD);
    if (fd2 < 0)
        { regress_error(RGE_OPEN); po_close(fd1); return(FALSE);}
    if (po_lseek(fd2, 512, PSEEK_SET) != 512)
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2); return(FALSE);}
    for (i = 0; i < 256; i++)
    {
        if ( (po_read(fd1, (byte *)&j, 2) != 2) || j != i)
            { regress_error(RGE_READ); po_close(fd1); po_close(fd2);return(FALSE);}
        if ( (po_read(fd2, (byte *)&j, 2) != 2) || j != i+256)
            { regress_error(RGE_READ); po_close(fd1); po_close(fd2);return(FALSE);}
    }
    /* Write buffered, read unbuffered */
    if ( (po_lseek(fd1, 0, PSEEK_SET) != 0) || (po_lseek(fd2, 0, PSEEK_SET) != 0))
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2);return(FALSE);}
    for (j = 99, i = 0; i < 512; i++)
        if (po_write(fd1, (byte *)&j, 2) != 2)
            { regress_error(RGE_WRITE); po_close(fd1); po_close(fd2);return(FALSE);}
    if (po_read(fd2, (byte *)wbuff, 1024) != 1024)
            { regress_error(RGE_READ); po_close(fd1); po_close(fd2);return(FALSE);}
    for (j = 99, i = 0; i < 512; i++)
        if (wbuff[i] != j)
            { regress_error(RGE_READ); po_close(fd1); po_close(fd2);return(FALSE);}
    /* Seek test */
    if ( (po_lseek(fd1, 520, PSEEK_SET) != 520) )
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2);return(FALSE);}
    j = 520; po_write(fd1, (byte *)&j, 2);
    if ( (po_lseek(fd1, 20, PSEEK_SET) != 20) )
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2);return(FALSE);}
    j = 20; po_write(fd1, (byte *)&j, 2);
    if ( (po_lseek(fd1, 520, PSEEK_SET) != 520) )
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2);return(FALSE);}
    j = 0; po_read(fd1, (byte *)&j, 2);
    if (j != 520)
        { regress_error(RGE_READ); po_close(fd1); po_close(fd2);return(FALSE);}
    if ( (po_lseek(fd1, 20, PSEEK_SET) != 20) )
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2);return(FALSE);}
    j = 0; po_read(fd1,(byte *) &j, 2);
    if (j != 20)
        { regress_error(RGE_READ); po_close(fd1); po_close(fd2);return(FALSE);}

    /* Write buffered and write unbuffered */
    if ( (po_lseek(fd1, 0, PSEEK_SET) != 0) || (po_lseek(fd2, 0, PSEEK_SET) != 0))
        { regress_error(RGE_SEEK); po_close(fd1); po_close(fd2);return(FALSE);}
    for (i = 0; i < 512; i++)
        wbuff[i] = (word) (512-i);
    for (i = 0; i < 512; i++)
        if (po_write(fd1, (byte *)&i, 2) != 2)
            { regress_error(RGE_WRITE); po_close(fd1); po_close(fd2);return(FALSE);}
    if (po_write(fd2, (byte *)wbuff, 1024) != 1024)
            { regress_error(RGE_WRITE); po_close(fd1); po_close(fd2);return(FALSE);}
    po_close(fd1);
    if ( po_lseek(fd2, 0, PSEEK_SET) != 0 )
        { regress_error(RGE_SEEK); po_close(fd2);return(FALSE);}
    for (i = 0; i < 512; i++)
        if ( (po_read(fd2, (byte *)&j, 2) != 2) || j != 512-i)
            { regress_error(RGE_READ); po_close(fd2); return(FALSE);}
    po_close(fd2);
    if (do_clean)
        pc_unlink(test_file_name);
    return(TRUE);
}


#define FOUR_GIG_BLOCKS 0x80000
#define ONE_MEG (dword) 0x100000
#define ONE_MEG_SHY (0x100000-512)
#define FILL_TEST_HIT_WRAP 0

BOOLEAN do_large_file_test(void)
{
int residual,fd;
dword i, max_filesize_megs,ltemp,ltemp2,target_value,/*nfree,*/ blocks_total,blocks_free;
ERTFS_STAT stat_buff;

    max_filesize_megs = RTFS_MAX_FILE_SIZE/ONE_MEG;
    ltemp = max_filesize_megs * ONE_MEG;
    ltemp2 =  (RTFS_MAX_FILE_SIZE-ltemp);
    residual = (int)ltemp2;
    /* Won't work on a 16 bit machine */
    if ((dword) residual != ltemp2)
        return(FALSE);

    if (residual == 0)
    {
        max_filesize_megs -= 1;
        residual = ONE_MEG;
    }

    /*nfree =*/ pc_free(test_drive, &blocks_total, &blocks_free);
    if (blocks_free <  0x80000) /* Not enough space to do long test */
        return(TRUE);
#if (VERBOSE)
    RTFS_PRINT_STRING_1(USTRING_RTFSDEM_17,PRFLG_NL); /* "Performing Large 4GIG File io test" */
#endif
    /* Test read, write, ulseek, chsize, extend file */
    fd = po_open(test_file_name, PO_RDWR|PO_CREAT|PO_EXCL, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!check_errno(0)) return(FALSE);
    test_rtfs_buf[0] = 0;
    /* Write the file - 1 meg at a time */
    for (i = 0 ; i < max_filesize_megs; i++)
    {
         if (po_write(fd, (byte *) test_rtfs_buf, 512) != 512)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
        if (po_write(fd, (byte *) 0, ONE_MEG_SHY) != ONE_MEG_SHY)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
        test_rtfs_buf[0] += ONE_MEG;
    }
    /* The last meg will be truncated */
#if(FILL_TEST_HIT_WRAP)
#if (RTFS_TRUNCATE_WRITE_TO_MAX)
    if (po_write(fd, (byte *) 0, residual+1) != residual)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_write(fd, (byte *) 0, 1) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#else
    if (po_write(fd, (byte *) 0, residual+1) != -1)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_write(fd, (byte *) 0, residual) != residual)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#endif
#else
    /* The last meg will be truncated */
    if (po_write(fd, (byte *) test_rtfs_buf, 512) != 512)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#if (RTFS_TRUNCATE_WRITE_TO_MAX)
    if (po_write(fd, (byte *) 0, ONE_MEG) != (residual-512))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#else
    if (po_write(fd, (byte *) 0, ONE_MEG) != -1)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_write(fd, (byte *) 0, (residual-512)) != (residual-512))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#endif
#endif
    /* Read it back - 1 gig at a time */
    if (!po_ulseek(fd, 0, &ltemp, PSEEK_SET))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Read the file - 1 gig at a time */
    target_value = 0;
    for (i = 0 ; i < max_filesize_megs; i++)
    {
        if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
            (test_rtfs_buf[0] !=  target_value) )
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
        target_value += ONE_MEG;
        if (po_read(fd, (byte *) 0, ONE_MEG_SHY) != ONE_MEG_SHY)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    }
    /* The last meg will be truncated */
#if (FILL_TEST_HIT_WRAP)
     if (po_read(fd, (byte *) 0, ONE_MEG) != residual)
     { regress_error(RGE_LARGEFILETEST); return(FALSE);}
     if (po_read(fd, (byte *) 0, ONE_MEG) != 0)
     { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#else
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
         (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_read(fd, (byte *) 0, ONE_MEG) != residual-512)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
#endif

#if (FILL_TEST_HIT_WRAP)
    po_close(fd);
    goto unlink_it;
#endif

    /* Test po_ulseek */
    /* Test seek end */
    if (!po_ulseek(fd, 0, &ltemp, PSEEK_END) || ltemp != RTFS_MAX_FILE_SIZE)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Test seek curr neg */
    target_value = (dword) (max_filesize_megs * ONE_MEG);
    if (!po_ulseek(fd, residual, &ltemp, PSEEK_CUR_NEG) || ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!po_ulseek(fd, 0, &ltemp, PSEEK_CUR_NEG) ||
        ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
        (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!po_ulseek(fd, 512, &ltemp, PSEEK_CUR_NEG) ||
        ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value -= (10 * ONE_MEG);
    if (!po_ulseek(fd, 10 * ONE_MEG, &ltemp, PSEEK_CUR_NEG) || ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
        (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!po_ulseek(fd, 512, &ltemp, PSEEK_CUR_NEG) ||
        ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Test seek curr positive */
    target_value += (4 * ONE_MEG);
    if (!po_ulseek(fd, 4 * ONE_MEG, &ltemp, PSEEK_CUR) || ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
        (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!po_ulseek(fd, 512, &ltemp, PSEEK_CUR_NEG) ||
        ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!po_ulseek(fd, 0, &ltemp, PSEEK_CUR) ||
        ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    /* Test seek set */
    target_value = (4 * ONE_MEG);
    if (!po_ulseek(fd, target_value, &ltemp, PSEEK_SET) || ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
        (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = ((max_filesize_megs - 1000) * ONE_MEG);
    if (!po_ulseek(fd, target_value, &ltemp, PSEEK_SET) || ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
        (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Test fstat set */
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != RTFS_MAX_FILE_SIZE)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Test stat set */
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != RTFS_MAX_FILE_SIZE)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Test reopen */
    fd = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (!po_ulseek(fd, 0, &ltemp, PSEEK_END) || ltemp != RTFS_MAX_FILE_SIZE)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = ((max_filesize_megs - 1000) * ONE_MEG);
    if (!po_ulseek(fd, target_value, &ltemp, PSEEK_SET) || ltemp != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if ( (po_read(fd, (byte *) test_rtfs_buf, 512) != 512) ||
        (test_rtfs_buf[0] !=  target_value))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    /* Test fstat */
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != RTFS_MAX_FILE_SIZE)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    /* Test chsize (truncate) to zero */
    target_value = 0;
    if (po_chsize(fd, target_value) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    /* Test chsize (expand) to RTFS_MAX_FILE_SIZE */
    fd = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = RTFS_MAX_FILE_SIZE;
    if (po_chsize(fd, target_value) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}


    /* Test chsize (expand) to RTFS_MAX_FILE_SIZE */
    fd = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = RTFS_MAX_FILE_SIZE;
    if (po_chsize(fd, target_value) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    /* Test chsize (truncate) to 4000 meg */
    fd = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = ((max_filesize_megs - 1000) * ONE_MEG);
    if (po_chsize(fd, target_value) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    /* Test chsize (truncate) to 1000 meg */
    fd = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = (1000 * ONE_MEG);
    if (po_chsize(fd, target_value) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    /* Test chsize (expand) to ffffffff0  */
    fd = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
    if (fd < 0)
        { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    target_value = RTFS_MAX_FILE_SIZE-16;
    if (po_chsize(fd, target_value) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_fstat(fd, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (po_close(fd) != 0)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}
    if (pc_stat(test_file_name, &stat_buff) != 0 || stat_buff.st_size != target_value)
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

#if (FILL_TEST_HIT_WRAP)
unlink_it:
#endif
    if (!pc_unlink(test_file_name))
    { regress_error(RGE_LARGEFILETEST); return(FALSE);}

    return(TRUE);
}


PCFD fdarray[250];
/* Test file manipulation routines */
BOOLEAN do_file_test(int loop_count, BOOLEAN do_clean)
{
int i;
int j;
dword index;
dword di;
int ntestfiles = prtfs_cfg->cfg_NUSERFILES;


    if (ntestfiles >= 250)
        ntestfiles = 250;

#if USEPRINTF
#if (!VERBOSE)
    RTFS_PRINT_STRING_1(USTRING_RTFSDEM_12,PRFLG_NL); /* "-" */
#endif
#endif
    if (!loop_count)
        if (!do_large_file_test())
            return(FALSE);
#if (VERBOSE)
    RTFS_PRINT_STRING_1(USTRING_RTFSDEM_13,PRFLG_NL); /* "Performing File io test" */
#endif
    fdarray[0] = po_open(test_file_name, PO_RDWR|PO_CREAT|PO_EXCL, PS_IWRITE|PS_IREAD);
    if (fdarray[0] < 0)
        { regress_error(RGE_OPEN); return(FALSE);}
    if (!check_errno(0)) return(FALSE);
    for (i = 1; i < ntestfiles;i++)
    {
        /* This should fail */
        fdarray[i] = po_open(test_file_name, PO_RDWR|PO_CREAT|PO_EXCL, PS_IWRITE|PS_IREAD);
        if (fdarray[i] >= 0)
            { regress_error(RGE_OPEN); return(FALSE);}
        if (!check_errno(PEEXIST)) return(FALSE);

        /* This should work */
        fdarray[i] = po_open(test_file_name, PO_RDWR, PS_IWRITE|PS_IREAD);
        if (fdarray[i] < 0)
            { regress_error(RGE_OPEN); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
    }
    /* Write into the file using all file descriptors */
    index = 0;
    for (i = 0; i < ntestfiles;i++)
    {
        if (po_lseek(fdarray[i], 0L, PSEEK_END) == -1L)
            { regress_error(RGE_SEEK); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
        for (j = 0; j < NLONGS; j++)
            test_rtfs_buf[j] = index++;
        if (po_write(fdarray[i], (byte *) test_rtfs_buf, (NLONGS*4)) != (NLONGS*4))
            { regress_error(RGE_WRITE); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
    }

    /* Read file using all fds */
    index = 0;
    for (i = 0; i < ntestfiles;i++)
    {
        if (po_lseek(fdarray[i], (dword) (index*4), PSEEK_SET) != (long) (index*4))
            { regress_error(RGE_SEEK); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
        if (po_read(fdarray[i], (byte *) test_rtfs_buf, (NLONGS*4)) != (NLONGS*4))
            { regress_error(RGE_READ); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
        for (j = 0; j < NLONGS; j++)
        {
            if (test_rtfs_buf[j] != index++)
                { regress_error(RGE_READ); return(FALSE);}
        }
    }

    /* This should fail if more than one file is open */
    if (ntestfiles > 1)
    {
        if (po_truncate(fdarray[0], 256))
            { regress_error(RGE_TRUNC); return(FALSE);}

        if (!check_errno(PEACCES)) return(FALSE);
    }
    if (!po_flush(fdarray[0]))
        { regress_error(RGE_FLUSH); return(FALSE);}
    if (!check_errno(0)) return(FALSE);

    /* Close all secondary files */
    for (i = 1; i < ntestfiles;i++)
    {
        if (po_close(fdarray[i]) != 0)
            { regress_error(RGE_CLOSE); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
    }
    /* This should work */
    if (!po_truncate(fdarray[0], 256))
        { regress_error(RGE_TRUNC); return(FALSE);}
    if (!check_errno(0)) return(FALSE);

    /* This should work */
    for (di = 0; di < 64 * 1024; di += 1024)
    {
        if (po_chsize(fdarray[0], di) != 0)
        { regress_error(RGE_CHSIZE); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
    }

    /* This should work */
    for (di = 63 * 1024; di; di -= 1024)
    {
        if (po_chsize(fdarray[0], di) != 0)
        { regress_error(RGE_CHSIZE); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
    }

    /* This should fail */
    if (pc_unlink(test_file_name))
        { regress_error(RGE_UNLINK); return(FALSE);}
    if (!check_errno(PEACCES)) return(FALSE);

    if (po_close(fdarray[0]) != 0)
        { regress_error(RGE_CLOSE); return(FALSE);}
    if (!check_errno(0)) return(FALSE);

    if (!pc_mv(test_file_name, test_newfile_name))
        { regress_error(RGE_MV); return(FALSE);}
    if (!check_errno(0)) return(FALSE);

    /* This should work */
    /* Manually delete on odd loops, even loops use deltree */
    if (do_clean && (loop_count & 1))
    {
        if (!pc_unlink(test_newfile_name))
            { regress_error(RGE_UNLINK); return(FALSE);}
        if (!check_errno(0)) return(FALSE);
    }

    return(TRUE);
}

BOOLEAN check_errno(int expected_error)
{
    if (get_errno() != expected_error)
    {
        regress_error(RGE_ERRNO);
        RTFS_PRINT_LONG_1((dword) get_errno(), PRFLG_NL);
        return(FALSE);
    }
    else
    {
        rtfs_set_errno(PEILLEGALERRNO); /* Set it to illegal, it should be cleared */
        return(TRUE);
    }
}

void regress_error(int error)                                   /*__fn__*/
{
    RTFS_PRINT_STRING_1(USTRING_RTFSDEM_14, 0); /* "regress_error was called with error" */
    RTFS_PRINT_LONG_1((dword) error, PRFLG_NL);
    return;
}
