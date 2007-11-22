/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren , 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* winhdisk.c - Simulate disk drive in a file on the host operating system

Summary

 Description
    Provides a device driver that reads and writes data to a file on the
    host disk. This version of the driver creates a file named
    HOSTDISK.DAT. This is targeted for hosts runnig Win95/NT but it can
    be migrated to other hosts.


    The format routine requires an initialized external variable
    named:
        extern word  host_disk_size;
    that tells it what size in blocks the file should be formatted to.


    This driver is used primarilly for the MKROM utility but it is
    fully functional and can be used for other purposes.

*/

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <stdlib.h>
#include <string.h>

#include "rtfs.h"
#include "portconf.h"   /* For included devices */

#if (INCLUDE_HOSTDISK)

/* hostdisk - Device driver that uses a DOS file as a virtual disk.

 Description
    Provides a virtual volume that resides in a (host) DOS file. This
    volume can be used to prototype applications. It can also be used to build
    the image of a rom disk which can then be accessed through the
    rom disk driver. (see winmkrom.c)

    To enable this driver set INCLUDE_HOSTDISK to 1 in portconf.h
*/


void pc_calculate_chs(dword total, dword *cylinders, int *heads, int *secptrack);

#define DEFAULT_HOST_DISK_SIZE 10240 /* 5M, FAT16 */

#define WINDOWS_HOSTDISK_PRINTF printf
#define WINDOWS_HOSTDISK_SPRINTF sprintf


#define MAXSEGMENTS_PER_UNIT 16
#define MAX_UNITS 8
struct file64 {
char basename[255];
int num_segments;
HANDLE segment_handles[MAXSEGMENTS_PER_UNIT];
};
BOOLEAN flush_all_writes;
struct file64 sixty_four_bit_volumes[MAX_UNITS];

/*  there are 0x200000 blokcs per gigabyte segment */

#define BLOCKS_PER_GIG 0x200000

int alloc_64bit_unit()
{
int i;
    for (i = 0 ;i < MAX_UNITS; i++)
    {
        if (!sixty_four_bit_volumes[i].num_segments)
            return(i);
    }
    return(-1);
}
int open_64bit_volume(int unit, byte *basename)
{
dword i;
char segment_file_name[256];

    rtfs_strcpy((byte *) &(sixty_four_bit_volumes[unit].basename[0]), basename);
    sixty_four_bit_volumes[unit].num_segments = 0;
    for (i = 0;;i++)
    {
        WINDOWS_HOSTDISK_SPRINTF(segment_file_name,"%s_SEGMENT_%x.HDK",basename,i);
        /* reopen the file segments */
        sixty_four_bit_volumes[unit].segment_handles[i] =
                CreateFile(TEXT(segment_file_name),    // file to open
                   GENERIC_READ|GENERIC_WRITE,
                   0,       // share for reading
                   NULL,                  // default security
                   OPEN_EXISTING,         // existing file only
                   FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING,
                   NULL);                 // no attr. template

        if (sixty_four_bit_volumes[unit].segment_handles[i] == INVALID_HANDLE_VALUE)
        {
            if (i == 0)
                return(-1); /* Nothing there */
            else
             return(0);
        }
        else
            sixty_four_bit_volumes[unit].num_segments += 1;
    }
}
int create_64bit_volume(int unit, byte *basename, dword nblocks)
{
dword i;
char segment_file_name[256];
    rtfs_strcpy((byte *)sixty_four_bit_volumes[unit].basename, (byte *)basename);
    sixty_four_bit_volumes[unit].num_segments = (nblocks + BLOCKS_PER_GIG - 1)/BLOCKS_PER_GIG;
    for (i = 0 ;i < (dword)sixty_four_bit_volumes[unit].num_segments; i++)
    {
        WINDOWS_HOSTDISK_SPRINTF(segment_file_name,"%s_SEGMENT_%x.HDK",basename,i);
        sixty_four_bit_volumes[unit].segment_handles[i] =
                CreateFile(TEXT(segment_file_name),    // file to open
                   GENERIC_READ|GENERIC_WRITE,
                   0,       // share for reading
                   NULL,                  // default security
                   CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING,
                   NULL);                 // no attr. template

        if (sixty_four_bit_volumes[unit].segment_handles[i] == INVALID_HANDLE_VALUE)
                return(-1);
    }
    return(0);
}
void close_64bit_volume(int unit)
{
dword i;
    for (i = 0 ;i < (dword)sixty_four_bit_volumes[unit].num_segments; i++)
        CloseHandle(sixty_four_bit_volumes[unit].segment_handles[i]);
}


dword size_64bit_volume(int unit)
{
DWORD size, s;

    if (!sixty_four_bit_volumes[unit].num_segments)
        return(0);

    size = (sixty_four_bit_volumes[unit].num_segments-1) * BLOCKS_PER_GIG;
    s = SetFilePointer(sixty_four_bit_volumes[unit].segment_handles[sixty_four_bit_volumes[unit].num_segments-1],
            0,0, FILE_END);
    if (s == INVALID_SET_FILE_POINTER)
        return(0);
    size += s/512;
    return((dword)size);
}

BOOLEAN hostdisk_io_64(int unit, dword block, void  *buffer, word _count, BOOLEAN reading) /*__fn__*/
{
dword segment_number;
dword max_count,block_offset,count;
dword s,ltemp;
HANDLE hFile;
dword nbytes,nblocks;
byte *bbuffer;
    bbuffer = (byte *) buffer;
    count = (dword) _count;
    while (count)
    {
        segment_number = block/BLOCKS_PER_GIG;
        block_offset = block%BLOCKS_PER_GIG;
        hFile = sixty_four_bit_volumes[unit].segment_handles[segment_number];
        ltemp = (dword)(block_offset * 512);
        s = SetFilePointer(hFile,
            ltemp,0, FILE_BEGIN);
        if (s == INVALID_SET_FILE_POINTER)
            return(FALSE);
        if (s != ltemp)
        {
            return(FALSE);
        }
        max_count = (BLOCKS_PER_GIG - block_offset);
        if (count > max_count)
        {
            nblocks = max_count;
        }
        else
        {
            nblocks = (int) count;
        }
        block += (dword)nblocks;
        count -= (dword)nblocks;
        nbytes = nblocks*512;
        if (reading)
        {
        dword nread;
            if (!ReadFile(hFile,bbuffer,nbytes,&nread,0) ||    nread != nbytes)
                return(FALSE);
        }
        else
        {
        dword nwrote;
            if (!WriteFile(hFile,bbuffer,nbytes,&nwrote,0) || nwrote != nbytes)
                return(FALSE);
        }
        bbuffer += nbytes;
    }
    return(TRUE);
}



/*
*
*   Perform io to and from the hostdisk.
*
*   If the reading flag is true copy data from the hostdisk (read).
*   else copy to the hostdisk. (write).
*
*/

extern dword simulate_disk_latency;
dword decrement(dword l);

BOOLEAN hostdisk_io(int driveno, dword block, void  *buffer, word count, BOOLEAN reading) /*__fn__*/
{
    DDRIVE *pdr;

    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr)
        return(FALSE);
    return(hostdisk_io_64(pdr->logical_unit_number, block, buffer, count, reading));
}

/* file is already truncated, this just expands it to correct size */
static BOOLEAN hostdisk_make_file(int driveno, dword numblocks)
{
    dword block;
    BLKBUFF *buf;
    BOOLEAN saved_flush_all_writes;
    dword segment;
    buf = pc_scratch_blk();
    if (!buf)
        return(FALSE);

    rtfs_memset(buf->data, 0, 512);
    block = 0;
    /* seek to the end of each one gig segment of the file and write one block
       (this will extend the file) */
    saved_flush_all_writes = flush_all_writes;
    flush_all_writes = TRUE;   /* don't change.. experimental doesn't speed things up */
    segment = 1;
    while (block < numblocks-1)
    {
        dword ltemp;
        ltemp = (segment * BLOCKS_PER_GIG) -1;
        if (ltemp >= numblocks)
            block = numblocks - 1;
        else
            block = ltemp;
        if (!hostdisk_io(driveno, block, buf->data, 1, FALSE))
        {
            flush_all_writes = saved_flush_all_writes;
            pc_free_scratch_blk(buf);
            return(FALSE);
        }
        segment += 1;
    }
    flush_all_writes = saved_flush_all_writes;
    pc_free_scratch_blk(buf);
    return(TRUE);
}

int hostdisk_perform_device_ioctl(int driveno, int opcode, void * pargs)
{
DDRIVE *pdr;

    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr)
        return (-1);

    switch (opcode)
    {
        case DEVCTL_GET_GEOMETRY:
        {
            DEV_GEOMETRY gc;

            rtfs_memset(&gc, 0, sizeof(gc));

            if (pdr->logical_unit_number < 0)
                return(-1);

            /* Now set the geometry section */
            /* This is a simple default condition. It is overriden by
            code in MKROM when the hostdisk is being populated from
            a disk sub-tree */
            gc.dev_geometry_lbas = size_64bit_volume(pdr->logical_unit_number);
            if (!gc.dev_geometry_lbas)
                return(-1);
            pc_calculate_chs(gc.dev_geometry_lbas, &gc.dev_geometry_cylinders, &gc.dev_geometry_heads, &gc.dev_geometry_secptrack);
            copybuff(pargs, &gc, sizeof(gc));
            return (0);
        }

        case DEVCTL_FORMAT:
        {
            /* Fill the file with zeroes */
            dword l;
            int fd, answer;
            PDEV_GEOMETRY pgc;
            byte buf[20];

            pgc = (PDEV_GEOMETRY) pargs;

            fd = pdr->logical_unit_number;
            /* The file should be open already */
            if (fd < 0)
                return(-1);

            l = (dword) pgc->dev_geometry_heads;
            l = (dword) l * pgc->dev_geometry_cylinders;
            l = (dword) l * pgc->dev_geometry_secptrack;
            if (pgc->dev_geometry_lbas)
                l = pgc->dev_geometry_lbas;

            answer = '1';
            do
            {
                if (answer != '1')
                {
                    WINDOWS_HOSTDISK_PRINTF("I didn't understand your choice.\n");
                }
                /* Here we want to prompt the user so that he or she can
                change the size of the hostdisk to several presets. */
                WINDOWS_HOSTDISK_PRINTF("What size do you want the hostdisk '%s' to be?\n", (char*)pdr->device_name);
                WINDOWS_HOSTDISK_PRINTF("    1) FAT12 (4M)\n"); /* 8192 blocks */
                WINDOWS_HOSTDISK_PRINTF("    2) FAT16 (16M)\n"); /* 32768 blocks */
                WINDOWS_HOSTDISK_PRINTF("    3) FAT32 (1G)\n"); /* 2097152 blocks */
                WINDOWS_HOSTDISK_PRINTF("    4) Current (~%iM)\n", l/2048);
                WINDOWS_HOSTDISK_PRINTF("    5) Custom\n");
                WINDOWS_HOSTDISK_PRINTF("    6) FAT32 (16G)\n"); /* 33554432 blocks */
                WINDOWS_HOSTDISK_PRINTF(": ");
                rtfs_port_tm_gets(buf);
                if (strlen((char*)buf) == 1)
                    answer = buf[0];
                else
                    answer = 'X'; /* wrong */
            }
            while (answer < '1' || answer > '6');
            if (answer == '5')
            {
                l = 1;
                do
                {
                    if (l == 0)
                    {
                        WINDOWS_HOSTDISK_PRINTF("I didn't understand your choice.\n");
                    }
                    WINDOWS_HOSTDISK_PRINTF("How many megabytes would you like the hostdisk to contain?\n");
                    WINDOWS_HOSTDISK_PRINTF(": ");
                    rtfs_port_tm_gets(buf);
                    l = atoi((char*)buf);
                }
                while (l == 0);
                l *= 2048;
            }
            switch (answer)
            {
            case '1': l = 8192; break;
            case '2': l = 32768; break;
            case '3': l = 2097152; break;
            case '6': l = 33554432; break;
            case '4': /* fall through */
            case '5': /* fall through */
            default: /* shouldn't happen */
                break;
            }
            WINDOWS_HOSTDISK_PRINTF("Making disk %iM.\n", l/2048);
            close_64bit_volume(fd);
            if (create_64bit_volume(fd, pdr->device_name, l) != 0)
                return(-1);
            if (!hostdisk_make_file(driveno, l))
                return(-1);

            /* Update caller's idea of geometry */
            pgc->dev_geometry_lbas = l;
            pc_calculate_chs(pgc->dev_geometry_lbas, &pgc->dev_geometry_cylinders, &pgc->dev_geometry_heads, &pgc->dev_geometry_secptrack);
            return(0);
        }
        break;
        case DEVCTL_REPORT_REMOVE:
            pdr->drive_flags &= ~DRIVE_FLAGS_INSERTED;
            return(0);
        case DEVCTL_CHECKSTATUS:
            if (pdr->drive_flags & DRIVE_FLAGS_INSERTED)
                return(DEVTEST_NOCHANGE);
            else
            {
                pdr->drive_flags |= DRIVE_FLAGS_INSERTED;
                return(DEVTEST_CHANGED);
            }
        case DEVCTL_WARMSTART:
        {
            flush_all_writes = TRUE; /* Enable flush after each write by default */
            pdr->logical_unit_number = alloc_64bit_unit();
            if (pdr->logical_unit_number < 0)
                return(-1);
            /* See if it already exists */
            if (open_64bit_volume(pdr->logical_unit_number, pdr->device_name) < 0)
            {
                pdr->drive_flags |= DRIVE_FLAGS_FORMAT;
                if (create_64bit_volume(pdr->logical_unit_number, pdr->device_name, DEFAULT_HOST_DISK_SIZE) != 0)
                    return(-1);
                if (!hostdisk_make_file(driveno, DEFAULT_HOST_DISK_SIZE))
                    return(-1);
            }
            pdr->drive_flags |= (DRIVE_FLAGS_VALID|DRIVE_FLAGS_INSERTED|DRIVE_FLAGS_REMOVABLE);
            return(0);
        }
            /* Fall through */
        case DEVCTL_POWER_RESTORE:
            /* Fall through */
        case DEVCTL_POWER_LOSS:
            /* Fall through */
        default:
            break;
    }
    return(0);

}

#endif /* (INCLUDE_HOSTDISK) */
