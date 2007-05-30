/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc, 1993-2002
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/*
*****************************************************************************
    MKROM.C   - Create a rom disk from an MS-DOS sudirectory 
                       
Summary

    MKROM subdir


 Description    
    Traverses the subdirectory "subdir" and make an exact image of that 
    subdirectory in a file named romdisk.dat. The file romdisk.dat
    can be placed directly into a rom and accessed via the rom disk
    device driver.
  
    


Example:

    mkrom c:\dev\romimage


*****************************************************************************
*/

#include <rtfs.h>
#include <portconf.h>   /* For included devices */
#if (INCLUDE_HOSTDISK)
/* Host disk  is required to use the MKROM utility */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <direct.h>
#include <string.h>

/* Write your own routines to create these. */
#define BIN_VOL_LABEL 0x12345678L  
#define STR_VOL_LABEL "VOLUMELABE" /* 11 chars max */

typedef struct _finddata_t statobj_t;

#define MKROM_MAX_PATH_LEN    260
#define FIXED_BLOCKS 2 /* Block zero and 1 reserved + 2 fudge */
#define CLUSTER_FUDGE 2

void drno_to_string(byte *pname, int drno); /* See rtfsinit.c */
int traverse(byte *name, int level);
int copy_files(byte *name, int level);
int make_directories(byte *name, int level);

byte imagepath[MKROM_MAX_PATH_LEN];

int  root_path_len = 0;    /* remember how much of each path is the root */

long required_volume_size = 0; /* Derived in calculate_format_parms */

int n_subdirectories = 0;      /* Total number of subdirectories */
int n_files = 0;               /* Total number of files    */
int  n_root_entries = 0;       /* Entries in the root */
int  n_sub_entries = 0;        /* Entries in subdirectories */
long file_cluster_count[33];        /* Count of files with */
long subdir_cluster_count[33];      /* Count of subdirs with */

BOOLEAN calculate_format_parms(DDRIVE *pdr, PDEV_GEOMETRY pgeometry);

BOOLEAN hostdisk_io(int driveno, dword block, void  *buffer, word count, BOOLEAN reading);

int domkhostdisk(int agc, byte **agv)                                      /*__fn__*/
{
int j,driveno;
byte drname[8];
byte inbuff[80];
DDRIVE *pdr;
DEV_GEOMETRY geometry;


    if (agc != 1)
    {
        printf("Usage MKHOSTDISK path\n");
        return(0);
    }
    else /* Get the path */
    {
#if (INCLUDE_CS_UNICODE)
        /* Convert unicode string to ASCII fro accessing win9x file system */
        map_unicode_to_ascii((byte *)&imagepath[0], (byte *) *agv);
#else
        strcpy((char *)&imagepath[0],(char *)*agv); 
#endif
    }

    n_subdirectories = 0;
    n_files = 0;               
    n_root_entries = 0;
    n_sub_entries = 0;
    for (j = 0; j< 33; j++)
    {
        file_cluster_count[j] = 0; 
        subdir_cluster_count[j] = 0;
    }

    driveno = -1;
    pdr= prtfs_cfg->mem_drives_structures;
    for (j = 0; j < prtfs_cfg->cfg_NDRIVES; j++, pdr++)
    {
        if (pdr->dev_table_drive_io == hostdisk_io)
        {
            drno_to_string(drname, pdr->driveno);
            driveno = pdr->driveno;
            break;
        }
    }

    if (driveno == -1 || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
    {
        printf("Fail: No host disk device is available\n");
        return 0;
    }

    /* Prepare to format the hostdisk */
    /* check media and clear change conditions */
    if (!check_drive_number_present(driveno))
    {
        printf("Check media failed can't format hostdisk.\n");
        return(0);
    }
    /* Get media parms, but override in next section */
    if (!pc_get_media_parms(drname, &geometry))
    {
        printf("Drive %s get media failed.\n", drname);
        return(0);
    }
    /* How long is the path to the root.. We'll strip this off of the
       front of the strings when we build RTFS files in the file image */
    root_path_len = rtfs_strlen(imagepath);


    printf("Populating host disk at %s from native subdirectory %s\n", drname, imagepath);

    traverse(imagepath, 0);
    if (!calculate_format_parms(pdr, &geometry))
        return(0);
    printf("\n");
    printf("\n");
    printf("Host Disk Image Will Contain\n");
    printf("\t %10.10d Files\n", n_files);
    printf("\t %10.10d Subdirectories\n", n_subdirectories);
    printf("\t requiring %10.10ld blocks\n", required_volume_size);
    printf("\n");
    printf("\n");
    printf("Type Y to create the Host Disk Image, any other key to exit\n");
    gets((char *)inbuff);
    printf("\n");
    if (inbuff[0] != 'Y' && inbuff[0] != 'y')
        return (0);
 
    if (!pc_format_media(drname, &geometry))
    {
       printf("Media format failed\n");
       return(0);
    }
    if (!pc_format_volume(drname, &geometry))
    {
       printf("Format volume failed\n");
       return(0);
    }

    pc_set_default_drive(drname);

    printf("Creating subdirectories\n");
    if (make_directories(imagepath, 0) < 0)
    {
        printf("Can not create subdirectories \n");
        return(0);
    }

    printf("Copying files\n");
    if (copy_files(imagepath, 0) < 0)
    {
        printf("Can not copy files\n");
        printf("Try adding extra blocks to the comand line\n");
        printf("Increment extra blocks untill it works\n");
        printf("For example: mkrom \\romdata 4\n");
        return(0);
    }
    printf("Host disk at %s is populated from native subdirectory %s\n", drname, imagepath);
    return(0);
}

/* Calculate number of clusters needed given cluster size */
long calc_n_clusters(long clsize)
{
long n_cl = 0;
int i;
    i = (int)clsize;
    n_cl += file_cluster_count[i];
    n_cl += subdir_cluster_count[i];
   return(n_cl);
}

long calc_best_cluster_size(void)
{
long min_clusters;
long best_size;
long try_size;
long l;

    best_size = 1;
    min_clusters = calc_n_clusters(best_size);
    for (try_size = 2; try_size <= 32; try_size *= 2)
    {
        l = calc_n_clusters(try_size);
        /* Take the minimum but limit it so we don't generate huge cluster 
           sizes on small volumes. We need at least a block anyway  */
        if (l < min_clusters && min_clusters > 256)
        {
            min_clusters = l;
            best_size = try_size;
        }
    }
    return(best_size);
}

void calculate_hcn(long n_blocks, PDEV_GEOMETRY pgeometry)
{
long cylinders;  /*- Must be < 1024 */
long heads;      /*- Must be < 256 */
long secptrack;  /*- Must be < 64 */
long residual_h; 
long residual_s; 

    secptrack = 1;
    while (n_blocks/secptrack > (1023L*255L))
        secptrack += 1;
    residual_h = (n_blocks+secptrack-1)/secptrack;
    heads = 1;
    while (residual_h/heads > 1023L)
        heads += 1;
    residual_s = (residual_h+heads-1)/heads;
    cylinders = residual_s;
    pgeometry->dev_geometry_cylinders = (dword) cylinders;
    pgeometry->dev_geometry_heads = (int) heads;
    pgeometry->dev_geometry_secptrack = (int) secptrack;
    printf("n_blocks == %ld hcn == %ld\n", n_blocks, (cylinders*heads*secptrack) );
}

BOOLEAN calculate_format_parms(DDRIVE *pdr, PDEV_GEOMETRY pgeometry)
{
long sec_p_alloc;
long n_clusters;
long nibs_p_cluster;
long nibs_p_fat;
long blocks_p_fat;
long blocks_p_root;
long blocks_p_heap;
long blocks_p_volume;

    rtfs_memset(pgeometry, 0, sizeof(*pgeometry));

    sec_p_alloc = calc_best_cluster_size();
    n_clusters = calc_n_clusters(sec_p_alloc);
    n_clusters += CLUSTER_FUDGE;
    blocks_p_heap = n_clusters * sec_p_alloc;

    if (n_clusters > 0xffffL)
    {
        nibs_p_cluster = 8;
        printf("Fat32 not supported for this function\n");
        return(FALSE);
    }
    else if (n_clusters < 4087)
        nibs_p_cluster = 3;
    else
        nibs_p_cluster = 4;

    nibs_p_fat = nibs_p_cluster * (n_clusters+2); /* Plus 2 for two reserved entries */
    blocks_p_fat = (nibs_p_fat+1023)/1024;
    blocks_p_root = (n_root_entries + 15)/16;
    n_root_entries = blocks_p_root*16;

    blocks_p_volume = FIXED_BLOCKS + blocks_p_heap + blocks_p_root + (2*blocks_p_fat);
    required_volume_size = blocks_p_volume;

    calculate_hcn(blocks_p_volume, pgeometry);

    rtfs_strcpy(&pgeometry->fmt.oemname[0],
                rtfs_strtab_user_string(USTRING_SYS_OEMNAME) );
    pgeometry->fmt.physical_drive_no =  (byte) pdr->driveno;
    pgeometry->fmt.binary_volume_label = BIN_VOL_LABEL;
    rtfs_strcpy(pgeometry->fmt.text_volume_label,
                rtfs_strtab_user_string(USTRING_SYS_VOLUME_LABEL) );
    pgeometry->fmt.secpalloc =      (byte)  sec_p_alloc;
    pgeometry->fmt.numfats     =    (byte)  2;
    pgeometry->fmt.secptrk     =    (word) pgeometry->dev_geometry_secptrack;
    pgeometry->fmt.numhead     =    (word) pgeometry->dev_geometry_heads;
    pgeometry->fmt.numcyl     =     (word) pgeometry->dev_geometry_cylinders;
    pgeometry->fmt.mediadesc   =    (byte)  0xF8;
    pgeometry->fmt.secreserved =    (word) 1;
    pgeometry->fmt.numhide     =    0;
    pgeometry->fmt.secpfat     =    (word) blocks_p_fat;
    pgeometry->fmt.numroot     =    (word) n_root_entries;
    pgeometry->fmt.mediadesc   =    (byte)  0xF8;
    pgeometry->fmt_parms_valid = TRUE;
    return(TRUE);
}


int traverse(byte *name, int level)
{
statobj_t statobj;
byte path[MKROM_MAX_PATH_LEN];
byte *pkind;
long n_blocks;
long n_clusters;
int  i;
int  entries;
long dd;
        
    for (i = 0; i < level; i++)
        printf(" ");
    printf("Processing directory %s\n", name);
    strcpy((char *)path, (char *)name);
    strcat((char *)path, "\\*.*");

    /* Print them all */
    if ((dd = _findfirst((char *)path, &statobj)) < 0)
    {
        return(0);
    }
    else
    {
        entries = 0;
        do 
        {
            entries++;
#if (VFAT) /* Small piece of compile time VFAT vs's NONVFAT code  */
            entries += (rtfs_strlen((byte *)statobj.name) + 12)/13; /* Account for lfn data */
#endif
            if ( (strcmp(".", statobj.name)==0) ||
                 (strcmp("..", statobj.name)==0)  )
                pkind = (byte *)"d";
            else if( statobj.attrib & _A_SUBDIR )
            {
                n_subdirectories += 1;
                pkind = (byte *)"d";
            }
            else
            {
                n_files += 1;
                n_blocks = (statobj.size+511)/512;
                for (i = 1; i <= 32; i *= 2)
                {
                    n_clusters = (n_blocks+i-1)/i;
                    file_cluster_count[i] += n_clusters;
                }
                pkind = (byte *)"-";
            }
            /* matches unix output. */
            for (i = 0; i < level; i++)
                printf(" ");
            printf("%s  %8ld  %-12s\r", pkind, statobj.size,statobj.name);
        } while (_findnext(dd, &statobj) == 0);
        _findclose(dd);
    }

    /* Track root entries and blocks used by dir ents */
    if (level == 0)
        n_root_entries += entries;
    else
    {
        n_sub_entries += entries;
        n_blocks = (entries + 15)/16;   /* Blocks occupied */
        for (i = 1; i <= 32; i *= 2)
        {
            n_clusters = (n_blocks+i-1)/i;
            subdir_cluster_count[i] += n_clusters;
        }
    }
    /* Now traverse */
    if ((dd=_findfirst((char *)path, &statobj))<0)
    {
        return(0);
    }
    else
    {
        do 
        {
            if( statobj.attrib & _A_SUBDIR )
            {
                if ( (strcmp(".", statobj.name)!=0) &&
                     (strcmp("..", statobj.name)!=0)  )
                {
                    strcpy((char *)path, (char *)name);
                    strcat((char *)path, "\\");
                    strcat((char *)path, statobj.name);
                    traverse(path, level+1);
                    strcpy((char *)path, (char *)name);
                    strcat((char *)path, "\\*.*");
                }
            }
        } while (_findnext(dd, &statobj) == 0);
        _findclose(dd);
    }
    return(1);
}


int make_directories(byte *name, int level)
{
statobj_t statobj;
byte path[MKROM_MAX_PATH_LEN];
byte *p;
long dd;
#if (INCLUDE_CS_UNICODE || !VFAT)
byte newpath[MKROM_MAX_PATH_LEN];
#endif

    if (level)
    {
        p = name + root_path_len;
#if (!VFAT) /* Small piece of compile time VFAT vs's NONVFAT code  */
        /* USE upper case for 8.3 */
        pc_str2upper(newpath, p);
        p = newpath;
#endif
        printf("Creating directory %s\n", p);
#if (INCLUDE_CS_UNICODE)
        map_ascii_to_unicode(newpath, p);
        if (!pc_mkdir(newpath) )
#else
        if (!pc_mkdir(p) )
#endif
        {
            printf("Failed creating directory %s\n", p);
            return(-1);
        }
    }

    strcpy((char *)path, (char *)name);
    strcat((char *)path, (char *)"\\*.*");

    if ((dd = _findfirst((char *)path, &statobj))<0)
    {
        return(0);
    }
    else
    {
        do 
        {
            if( statobj.attrib & _A_SUBDIR )
            {
                if ( (strcmp(".", statobj.name)!=0) &&
                     (strcmp("..", statobj.name)!=0)  )
                {
                    strcpy((char *)path, (char *)name);
                    strcat((char *)path, (char *)"\\");
                    strcat((char *)path, (char *)statobj.name);
                    if (make_directories(path, level+1) != 0)
                        return(-1);
                    strcpy((char *)path, (char *)name);
                    strcat((char *)path, (char *)"\\*.*");
                }
            }
        } while (_findnext(dd, &statobj) == 0);
        _findclose(dd);
    }
    return(0);
}

/* Copy Native file *from to dos path *to */
BOOLEAN copy_one_file(byte *path, byte *filename)            /* __fn__ */
{
static byte dos_path[MKROM_MAX_PATH_LEN];
static byte buf[1024];
PCFD  fd;
int res,res2;
int fi;
BOOLEAN retval = TRUE;
byte *to;
#if (INCLUDE_CS_UNICODE || !VFAT)
byte newpath[MKROM_MAX_PATH_LEN];
#endif


    strcpy((char *)dos_path, (char *)path);
    strcat((char *)dos_path, (char *)"\\");
    strcat((char *)dos_path, (char *)filename);

    /* Strip the leading path when we go to rtfs */
    to = &dos_path[root_path_len];
#if (!VFAT)/* Small piece of compile time VFAT vs's NONVFAT code  */
/* USE upper case for 8.3 */
        pc_str2upper(newpath, to);
        to = newpath;
#endif
    printf("Copying %s to %s\n", dos_path, to);
    if ( (fi = open((char *)dos_path,O_RDONLY|O_BINARY)) < 0)    /* Open binary read */ 
    {
        printf("Cant open %s\n",dos_path);
        return (FALSE);
    }
    else
    {
        /* Open the PC disk file for write */
#if (INCLUDE_CS_UNICODE)
        map_ascii_to_unicode(newpath, to);
        if ((fd = po_open(newpath,(PO_BINARY|PO_RDWR|PO_CREAT|PO_TRUNC),
                            (PS_IWRITE | PS_IREAD) ) ) < 0)
#else
        if ((fd = po_open(to,(PO_BINARY|PO_RDWR|PO_CREAT|PO_TRUNC),
                             (PS_IWRITE | PS_IREAD) ) ) < 0)
#endif
        {
            printf("Cant open %s error = %i\n",to, -1);
             return (FALSE);
        }
        else
        {
            /* Read from host until EOF */
            while ( (res = read(fi,&buf[0],1024)) > 0)
            {
                /* Put to the drive */
                if ( (res2 = (int)po_write(fd,buf,(word)res)) != res)
                {
                    printf("Cant write %x %x \n",res,res2);
                    retval = FALSE;
                    break;
                }
            }
            /* Close the pc file flush buffers &  and de-allocate structure */
            po_close(fd);

            /* close the native file */
            /* PORT-ME */
            close(fi);

            return (retval);
        }
    }
}    
 

int copy_files(byte *name, int level)
{
statobj_t statobj;
byte path[MKROM_MAX_PATH_LEN];
long dd;

    printf("Processing directory %s\n", name);
    strcpy((char *)path, (char *)name);
    strcat((char *)path, (char *)"\\*.*");

    if ((dd = _findfirst((char *)path, &statobj))>=0)
    {
        do 
        {
            if(!(statobj.attrib & (_A_SUBDIR) ))
            {
                if (!copy_one_file(name, (byte *)statobj.name))
                    return(-1);
            }
        } while (_findnext(dd, &statobj) == 0);
        _findclose(dd);
    }

    /* Now traverse */
    if ((dd = _findfirst((char *)path, &statobj)) < 0)
    {
        return(0);
    }
    else
    {
        do 
        {
            if( statobj.attrib & _A_SUBDIR )
            {
                if ( (strcmp(".", statobj.name)!=0) &&
                     (strcmp("..", statobj.name)!=0)  )
                {
                    strcpy((char *)path, (char *)name);
                    strcat((char *)path, (char *)"\\");
                    strcat((char *)path, statobj.name);
                    if (copy_files((byte *)path, level+1) < 0)
                        return(-1);
                    strcpy((char *)path, (char *)name);
                    strcat((char *)path, (char *)"\\*.*");
                }
            }
        } while (_findnext(dd, &statobj) == 0);
        _findclose(dd);
    }
    return(0);
}

int domkrom(int agc, byte **agv)                                      /*__fn__*/
{
FILE *infile, *outc;
int c, linecount=0;
long count=0;

    printf("\n MKROM - Creating drromdsk.h from hostdisk.dat\n");

    RTFS_ARGSUSED_INT(agc);
    RTFS_ARGSUSED_PVOID((void *) agv);
    infile = fopen("hostdisk.dat", "rb");
    if (infile == NULL)
    {
        printf("MKROM could not open hostdisk.dat\n");
        return(0);
    }

    outc = fopen("drromdsk.h", "wt");
    if (outc == NULL)
    {
        printf("MKROM could not open drromdsk.h\n");
        fclose(infile);
        return(0);
    }

    fprintf(outc, "/* %s */\n", "drromdsk.h");
    fprintf(outc, "/* Automatically Created from %s using MKROM */\n", "hostdisk.dat");
    fprintf(outc, "\nbyte KS_FAR %s[]=\n{\n", "romdisk_data");

    /* write the C file */
    while ((c=fgetc(infile)) != EOF) 
    {
        if (linecount < 16) 
        {
            fprintf(outc, " 0x%02x, ",c);
            linecount++;
        }
        else 
        {
            fprintf(outc, " 0x%02x,\n", c);
            linecount=0;
        }

        count++;
    }

    fprintf(outc, "};\n");

    fclose(infile);
    fclose(outc);
    return(0);
}

#endif /* (INCLUDE_HOSTDISK) */

