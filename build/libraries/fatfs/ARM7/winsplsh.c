/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1996
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* WINSPLSH.C - Introductory text and display engine for windows based demo */

#include <rtfs.h>
#include <portconf.h>   /* For included devices */

#if (INCLUDE_HOSTDISK)

#include <stdio.h>
#include <stdlib.h>

void display_introduction(void);

void display_text(char * readme[])
{
int nlines;
int current_line;
int break_line;
int i,j;
byte buf[32];
#define LINES_PER_PAGE 20

    /* Count number of lines */
    nlines = 0;
    while (readme[nlines])
        nlines++;

    current_line = 0;
    for (;;)
    {
        j = current_line;
        break_line = 0;
        for (i = 0; i < LINES_PER_PAGE; i++)
        {
            if (readme[j])
            {
                if (*readme[j] == '.')/* CDFS */
                    break_line = j;
                if (break_line)
                    printf("\n");
                else
                    printf("%s\n", readme[j++]);
            }
            else
                printf("\n");
        }
        printf("                            \n");
        /* "<return> for next,  (P)<return> for Previous, (Q)<return> to run demo" */
        rtfs_print_prompt_user(UPROMPT_WINSPLSH, buf);
        if (CS_OP_CMP_ASCII(buf,'Q') || CS_OP_CMP_ASCII(buf,'q'))
            break;
        if (CS_OP_CMP_ASCII(buf,'P') || CS_OP_CMP_ASCII(buf,'p'))
        {
            current_line -= LINES_PER_PAGE;
            if (current_line < 0)
                current_line = 0;
        }
        else
        {
            if (break_line)
             current_line = break_line + 1;
            else
            {
                if ((current_line + LINES_PER_PAGE) <= nlines)
                    current_line += LINES_PER_PAGE;
            }
        }
    }
}


char * rtfsdemo_introduction[] = {
" ",
" ",
" ",
" ",
" ",
" ",
" ",
" ",
" ",
"           ==================================================",
"           =     Thank you for your interest in ERTFS.      =",
"           =                                                =",
"           = Please Page through the following introduction =",
"           = At any time press Q to run the Demo.           =",
"           =                     ==                         =",
"           = The DEMO is safe to execute on your workstation=",
"           = it uses only ram disks, rom disks and a simple =",
"           = user file. It does NOT bypass windows to access=",
"           = your hard disk directly                        =",
"           ==================================================",
" ",
" ",
" ",
" ",
"    ERTFS is a DOS and WINDOWS compatible file system library for ",
"    embedded systems. ERTFS supports all versions of DOS, 12, 16 ",
"    and 32 bit File Allocation Tables. ERFTS even supports WIN95 long ",
"    file names, and large FAT32 partions of 80 gigabytes and more. ",
" ",
" ",
"    ERTFS has provided high performance reliable solutions for demanding ",
"    embedded Applications since 1988. It is used in hundreds of applications",
"    including medical devices, avionics, military, set-tops, digital cameras",
"    scientific instruments and many other devices. ",
" ",
"    ERTFS is portable, easy to use and has a familiar file system interface ",
"    but it is also a high performance disk subsystem that has been used in ",
"    many demanding and sophisticated applications including demanding video ",
"    audio and scientific applications.",
" ",
/* ". page break", */
"                       Advanced Features of ERTFS",
"    ",
"    . Callable check disk              . Callable format and fdisk functions",
"    . Contiguous Files                 . Automounts removable media   ",
"    . Direct device driver access      . Query file extents",
"    . User control over block placement. Mountable device drivers",
"    . Extensive device support         . Multiple partition support",
"    . Supports very large volumes      . CD ISO 9660 supported (optional)",
"    . Includes PCMCIA Support          . Clean RTOS and CPU porting layer",
"    . User placement of file extents   . Complete access to free space ",
"    . Full Posix style API",
"                                         ",
"                       Devices Supported by ERTFS",
"    ",
"    . ATA hard drives. Drives as large as 80 GigaBytes have been tested",
"    . Linear flash. FTL is provided with ram emulation of flash",
"    . PCMCIA Ram cards .  RAM disk  . Floppy disk . Smart Media   ",
"    . PCMCIA ATA disks and compact flash",
"    . ROM disk, includes tool to create a rom disk image from MS-Windows ",
"    . Table driven device interface supports user supplied device drivers,",
"      user devices may be removable and may have partitions. ERTFS handles",
"      all devices the same way.",
" ",
" ",
/* ". page break", */
"                       API Calls provided with ERTFS",
" ",
"                       -- Initilaization --",
"    pc_ertfs_init() - mount devices from device driver table. ",
" ",
"                       -- File Operations --",
"    po_open           - Open a file.",
"    po_read           - Read bytes from a file.",
"    po_write          - Write Bytes to a file.",
"    po_lseek          - Move the file pointer.",
"    po_close          - Close a file and flush the file allocation table.",
"    po_flush          - Flush an open file",
"    po_trunc          - Truncate an open file",
"    pc_mv             - Rename a file.",
"    pc_unlink         - Delete a file.",
"    po_chsize         - Truncate or extend an open file.",
"    pc_set_attributes - Set File Attributes  ",
"    pc_get_attributes - Get File Attributes  ",
"    pc_stat           - Obtain statistics on a path.",
"    pc_fstat          - Obtain statistics on an open file",
" ",
"                       -- Directory  Operations --",
"    pc_mkdir          - Create a directory.",
"    pc_rmdir          - Delete a directory.",
"    pc_deltree        - Delete an entire directory tree.",
"    pc_gfirst         - Get stats on the first file to match a pattern.",
"    pc_gnext          - Get stats on the next file to match a pattern.",
"    pc_gdone          - Free resources used by pc_gfirst/pc_gnext.",
" ",
"                       -- Miscelaneous Functions -- ",
"    chkdsk            - check file system integrity",
"    pc_fat_size       - Calculate blocks required for a volume's FAT",
"    pc_free           - Calculate and return the free space on a disk.",
"    pc_set_default_drive - Set the default drive number.",
"    pc_set_cwd        - Set the current working directory.",
"    pc_isdir          - Determine if a path is a directory.",
"    pc_isvol          - Determine if a path is a volume",
"    pc_pwd            - Get string representation of current working dir.",
"    pc_set_pwd        - Set current working directory for this user",
"    pc_setdfltfrvno   - Set the current default drive for this user.",
"    pc_enumerate      - Select all directory entries that match rules and",
"                        call a user callback on each. (like the unix print",
"                        utility)",
" ",
" ",
"                       -- Advanced High performance Functions -- ",
"    pc_cluster_size    - Get the cluster size of a drive",
"    pc_find_contig_clusters",
"                       - Find contiguous clusters in the freelist",
"    pc_get_free_list   - Get a list free cluster segments on the drive",
"    pc_raw_write       - Write blocks directly to a disk",
"    pc_raw_read        - Read blocks directly from a disk",
"    pc_get_file_extents- Get the list of block segments that make up a file",
"    po_extend_file     - Extend a file with contiguous clusters. Also ",
"                         supports direct assignment blocks to a file",
" ",
"                       -- Formatting routines -- ",
"    pc_get_media_parms - Get media parameters.",
"    pc_format_media    - Device level format",
"    pc_partition_media - Write partition table",
"    pc_format_volume   - Format a volume",
" ",
/* ". page break", */
"                       Sample utilities provided with ERTFS",
" ",
"    Test Shell - An interactive test shell is provided. This test",
"    shell is an excellent demontration of how to use the API as well ",
"    as a useful tool itself. The console IO needs of the test shell are ",
"    minimal and can be accomplished from a simple RS232 connection.",
" ",
"    Regression Test - This program tests the ERTFS API and is a good test ",
"    prgram for checking out ports of the library to new environments.",
" ",
"    Check disk - This is a utility and callable function that performs",
"    the check disk functionality.",
" ",
/* ". page break", */
"                       This demo program",
" ",
" ",
"  This demo program is built with the ERTFS library. It contains the ",
"  interactive test shell, the regression test module, and the check",
"  disk utility. ",
"  The demo runs from inside a \"DOS Box\" and it is safe to run it on your ",
"  workstation. All devices are simulated in ram or in simple disk files ",
"  in the current directory.",
" ",
/* ". page break", */
" ",
"  When the program starts you will see the following diagnostics.",
" ",
"ERTFS Device List",
"=================",
"Drive: A Device: HOST DISK HOSTDISK.DAT:256K",
"Drive: B Device: STATIC RAM DISK",
"Drive: C Device: FLASH DISK",
"Drive: D Device: STATIC ROM DISK",
" ",
"Notes:",
"1. Host disk is a 256 K disk volume simulated in a disk file",
"2. Flash Disk is the FTL driver running on a flsh disk simulation in ram",
"3. The Static Rom disk was initialized with the MKROM tool.",
" ",
"       =         ",
" ",
/* ". page break", */
" ",
"The program will then print:",
" ",
"Autoformatting RAM Devices",
"==========================",
"Autoformatting Drive: A Device: HOST DISK HOSTDISK.DAT:256K",
"Autoformatting Drive: B Device: STATIC RAM DISK",
"Autoformatting Drive: C Device: FLASH DISK",
"             =         ",
"The devices are autoformatted at initialization, an attribute in the ",
"device record tells ERTFS to format the device. Other attributes",
"flag the device as removable and partitioned.",
" ",
"Note that the ROM disk is not autoformatted at initialization time.",
" ",
" ",
/* ". page break", */
" ",
"The program next enters the test shell.",
"The test shell is like the DOS or Unix command shell. ",
"The Static ROM disk D: is already populated",
"so running informations commands on D: is a the most interesting",
"until you use the shell to populate the other volumes.",
" ",
"The test shell commands are listed on the next page.",
" ",
/* ". page break", */
" ",
"These commands are avaliable from the test shell: ",
"HELP",
"CAT PATH",
"CHSIZE FILENAME NEWSIZE",
"CD PATH or CD to display PWD ",
"CHKDSK D: <0,1> 1 is write lost chains",
"CLOSE FDNO",
"COPY PATH PATH",
"DELETE PATH",
"DELTREE PATH",
"DEVINFO (Display Device Information)",
"DIFF PATH PATH",
"DIR PATH",
"DSKSEL D:",
"ECHO: [args]",
"EJECT (ejects LS-120)",
"FILLFILE PATH PATTERN NTIMES",
"FORMAT (routine will prompt for arguments)",
/* ". page break", */
"GETATTR FILE",
"HELP:",
"LSTOPEN (lists all open file decscriptors) ",
"MKDIR PATH",
"PCMCIAINT (Force a PCMCIA mgmt Interrupt)",
"QUIT",
"READ FDNO",
"RENAME PATH NEWNAME",
"RMDIR PATH",
"RNDOP PATH RECLEN",
"SEEK FDNO RECORD",
"SETATTR D:PATH RDONLY|HIDDEN|SYSTEM|ARCHIVE|NORMAL",
"STAT PATH",
"WRITE FDNO QUOTED DATA",
"REGRESSTEST D: (perform regression test)",
">: ",
" ",
" ",
" ",
" ",
" Notes about using the demo test shell.                         ",
" 1. Since the D: drive is populated running chkdsk on it is most",
"    interesting. ",
" 2. To create a file on one of the other drives (A: to C:)      ",
"    use fillfile, rndop or copy files from the rom disk         ",
"    Example: ",
"     >: FILLFILE mydatafile.dat \"This is my story\" 100           ",
"     >: CAT mydatafile.dat ",
"    Will produce a list of 100 lines of \"This is my story\"",
"    Example: ",
"     >: rndop myfile.dat 100",
"     >: lstopen",
"    0: myfile.dat",
"     >: dir *.*",
"    myfile  .dat          0       03-28-88 19:37  ",
"           1      File(s)   252      KBytes free",
"     >: write 0 \"this is record zero\"",
"     >: write 0 \"this is record one\"",
"     >: write 0 \"this is record two\"",
"     >: seek 0 1",
"     >: read 0",
"    this is record one                                                                                  :",
"     >: close 0",
"     >: dir *.*",
"    myfile  .dat        300       03-28-88 19:37  ",
"           1      File(s)   252      KBytes free",
"     >: copy myfile.dat COPY.DAT",
"     >: dir *.*",
"    myfile  .dat        300       03-28-88 19:37  ",
"    COPY    .DAT        300       03-28-88 19:37  ",
"           2      File(s)   251      KBytes free",
"     >:",
" ",
" ",
"Thanks again for your interest in ERTFS.",
0,
};

void display_introduction(void)
{
       display_text(rtfsdemo_introduction);
}

void winexit(void)
{
    printf("========================================\n");
    printf("========================================\n");
    printf("========================================\n");
    printf("Thanks for trying ERTFS. Please Call If \n");
    printf("       You Have Any Questions           \n");
    printf("========================================\n");
    printf("========================================\n");
    printf("========================================\n");
    printf("========================================\n");
    exit(0);
}


#endif /* USEWIN32 - End readme display package */

