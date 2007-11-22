/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2006
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/

/* APIFASTMV.C - Contains user api level source code and some system level code

    The following API routines are included:

    pc_fast_mv      - Move a file to a filename in another subdirectoy when you know
                      the destination filename does not already exist.

    The following internal routines are included:

    static DROBJ *pc_fast_mknode() - This is a customized version of pc_mknode(), called only
    by pc_fast_mv(). It increaes performance by using an alias buffer and by not recycling deleted
    directory enteries.

    static BOOLEAN pc_fast_insert_inode() - This is a customized version of pc_insert_inode that
    increaes performance by using an alias buffer and by not recycling deleted directory enteries.

    static BLOCKT pc_fast_l_start_block() - This routine is used by pc_insert_inode() to first seek to the
    last cluster in a subdirectory before it executes the insertion algorithm.

*/
#include <rtfs.h>



/*  ALIAS_BUFFER


    You may pas an alias buffer into pc_fast_mv() to decrease the time spent
    calculating alias names.

    Alias buffer has 3 fields:

    byte *alias_buffer_data  - A buffer supplied by the user to hold alias
    file names. Each alias file name occupies 11 bytes so for example if you
    are populating a directory with 1000 files alias_buffer_data should point
    to a buffer containing at least 11000 bytes (11 * 1000)

    int alias_buffer_size    - Set by the user to tell pc_fast_mv how many
    8.3 file names can be stored at the memory address in alias_buffer_data

    int alias_buffer_count   - Must be set by the user to zero before the first
    call to pc_fast_mv. pc_fast_malias() increments this field as alias filenames
    are created.
*/

typedef struct alias_buffer
{
    int alias_buffer_size;
    int alias_buffer_count;
    byte *alias_buffer_data;
} ALIAS_BUFFER;


#define INCLUDE_FAST_TESTCODE 0/* Set to 1 to include test code at end of this file */

static DROBJ *pc_fast_mknode(ALIAS_BUFFER *palias, DROBJ *pmom ,byte *filename, byte *fileext, byte attributes, CLUSTERTYPE incluster);

static BOOLEAN pc_fast_insert_inode(ALIAS_BUFFER *palias, DROBJ *pobj , DROBJ *pmom, byte attr, CLUSTERTYPE initcluster, byte *filename, byte *fileext);
static BLOCKT pc_fast_l_start_block(DDRIVE *pdrive, BLOCKT curblock);

/***************************************************************************
    pc_fast_mv -  Move a file to a subdirectory

 Description:

    Move the file passed in paramteter old_name to new_name.

    BOOLEAN pc_fast_mv(
        ALIAS_BUFFER *pabuff,
        byte *old_name,
        byte *new_name)

        pabuff   - Optional alias buffer to improve vfat performance (see below)
        old_name - Path to source file (ex: "\\today\\file1.jpg")
        new_name - Path to destination file (ex: "\\archive\\file1.jpg")


    This subroutine saves execution time by:
     1. not checking if the the file already exists in the target subdirectory
     2. not recycling deleted subdirectory entries
     3. using a user supplied buffer to buffer file alias names so it can bypass
        scanning the subdirectory when calculating VFAT alias names

    The following condition must be must be met or pc_fast_mv may populate
    the target subdirectory with duplicate file names.

    1. The target subdirectory must not already contain a file that matches
       the filename component of new_name.


    See above for a description of the alias buffer

    Notes: About alias buffer:
    1. If no alias buffer is provided pc_fast_mv() will still work correctly but
    without alias name creation performance improvements.
    2. If the number of files created exceeds alias_buffer_size, the pc_fast_mv()
    will still work correctly but without alias name creation performance improvements.


    This example initializes an alias buffer that can be used to accelerate
    creation of up to 1000 directory entries.

static byte alias_data[ABUFFSIZE*11];
ALIAS_BUFFER a_buff;

    a_buff.alias_buffer_size  = ABUFFSIZE;
    a_buff.alias_buffer_count = 0;
    a_buff.alias_buffer_data = &alias_data[0];


    Fails if old_name is invalid

 Returns
    Returns TRUE if the file was moved. Or FALSE if the name not found.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid or they are not the same
    PEINVALIDPATH   - Path specified by old_name or new_name is badly formed.
    PEACCESS        - File or directory in use, or old_name is read only
    PEEXIST         - new_name already exists
    An ERTFS system error
***************************************************************************/


BOOLEAN pc_fast_mv(ALIAS_BUFFER *pabuff, byte *old_name, byte *new_name)  /*__apifn__*/
{
    int old_driveno;
    DROBJ *old_obj;
    DROBJ *old_parent_obj;
    byte  *path;
    byte  *filename;
    byte fileext[4];
    int new_driveno;
    DROBJ *new_obj;
    DROBJ *new_parent_obj;
    BOOLEAN ret_val;
    CLUSTERTYPE  cluster;
    DDRIVE *pdrive;

    int  p_errno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */
    p_errno = 0;

    /* Drives must be the same */
    if (    !pc_parsedrive( &old_driveno, old_name ) ||
            !pc_parsedrive( &new_driveno, new_name ) ||
            old_driveno != new_driveno)
    {
        rtfs_set_errno(PEINVALIDDRIVEID);
        return(FALSE);
    }

    /* Get the drive and make sure it is mounted   */
    old_driveno = check_drive_name_mount(old_name);
    if (old_driveno < 0)
    {
        /* errno was set by check_drive */
        return(FALSE);
    }
    rtfs_set_errno(0);  /* pc_mv: clear error status */

    old_obj         = 0;
    old_parent_obj  = 0;
    new_obj         = 0;
    new_parent_obj  = 0;
    ret_val = FALSE;

    pdrive = pc_drno2dr(old_driveno);

    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    path = (byte *)&(pdrive->pathname_buffer[0]);
    filename = (byte *)&(pdrive->filename_buffer[0]);

    /* Get out the filename and d:parent */
    if (!pc_parsepath(path, filename,fileext,old_name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Find the parent and make sure it is a directory  */
    old_parent_obj = pc_fndnode(path);
    if (!old_parent_obj)
        goto errex; /* pc_fndinode - set errno */

    if (!pc_isadir(old_parent_obj))
    {
        p_errno = PENOENT;
        goto errex;
    }
    /* Find the file */
    old_obj = pc_get_inode(0, old_parent_obj,
        filename, (byte*)fileext, GET_INODE_MATCH);
    if (!old_obj)
        goto errex; /* pc_get_inode - set errno */

     /* Be sure it exists and is a normal directory or file and is not open */
    if (pc_isroot(old_obj) || (old_obj->finode->opencount > 1) ||
       (old_obj->finode->fattribute&(ARDONLY|AVOLUME)))
    {
        p_errno = PEACCES;
        goto errex;
    }

    /* At this point old_obj contains the file we are renaming */

    /* Unlike pc_mv, pc_fast_mv does not test if the new directory entry already exists */
    rtfs_set_errno(0);  /* pc_mv - clear errno condition after failed pc_fndnode */

    /* Get out the filename and d:parent */
    if (!pc_parsepath(path,filename,fileext,new_name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Find the parent and make sure it is a directory  */
    new_parent_obj = pc_fndnode(path);
    if (!new_parent_obj || !pc_isadir(new_parent_obj) ||  pc_isavol(new_parent_obj))
        goto errex;


    /* The cluster value old */
    cluster = pc_finode_cluster(old_obj->pdrive,old_obj->finode);

    /* Create the new entry and assign cluster to it. If it is a directory
       .. will be linked correctly */
    new_obj = pc_fast_mknode(pabuff, new_parent_obj, filename, fileext, old_obj->finode->fattribute, cluster);
    if (!new_obj)
        goto errex;

    /* Copy the old directory entry stuf over */
    new_obj->finode->fattribute = old_obj->finode->fattribute;
    new_obj->finode->ftime = old_obj->finode->ftime;
    new_obj->finode->fdate = old_obj->finode->fdate;
    new_obj->finode->fsize = old_obj->finode->fsize;

    /* Update the new inode. Do not set archive bit or change date */
     if (!pc_update_inode(new_obj, FALSE, FALSE))
        goto errex;

    /* Set the old cluster value to zero */
    pc_pfinode_cluster(old_obj->pdrive,old_obj->finode,0);
    /* Delete the old but won't delete any clusters */
    if (!pc_rmnode(old_obj))
        goto errex;

    p_errno = 0;
    ret_val = TRUE;

    /* Good conditions fall through here, error exits jump to here */
errex:
    if (old_parent_obj)
        pc_freeobj(old_parent_obj);
    if (old_obj)
        pc_freeobj(old_obj);
    if (new_parent_obj)
        pc_freeobj(new_parent_obj);
    if (new_obj)
        pc_freeobj(new_obj);
    /* Set errno if we have one and not set by lower level already */
    if ((p_errno) && !get_errno())
        rtfs_set_errno(p_errno);
    if (!release_drive_mount_write(old_driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return(ret_val);
}


static BLOCKT pc_fast_l_start_block(DDRIVE *pdrive, BLOCKT curblock)             /*__fn__*/
{
    CLUSTERTYPE cluster;
    CLUSTERTYPE next_cluster;
       /* If the block is in the root area start from the first block
       blocks are preallocated so we don't know which is the last used */
    if (curblock < pdrive->firstclblock)
    {
        if (curblock < pdrive->rootblock)
        {
            rtfs_set_errno(PEINVALIDBLOCKNUMBER); /* pc_l_next_block: Internal error */
            return (BLOCKEQ0);
        }
        return (curblock);
    }
    else  /* In cluster space   */
    {
        /* In cluster space return the first block of the last cluster allocated
           we will scan up from here to find the end of the directory */
        if (curblock >= pdrive->numsecs)
        {
            rtfs_set_errno(PEINVALIDBLOCKNUMBER); /* pc_l_next_block: Internal error */
            return (BLOCKEQ0);
        }
        /* Get the next block   */
       /* Get the first cluster number */
        cluster = pc_sec2cluster(pdrive,curblock);
        if ((cluster < 2) || (cluster > pdrive->maxfindex) )
        {
            rtfs_set_errno(PEINVALIDBLOCKNUMBER); /* pc_l_next_block: Internal error */
            return (BLOCKEQ0);
        }

        do
        {
            next_cluster = FATOP(pdrive)->fatop_clnext(pdrive, cluster); /* Directory  */
            if (next_cluster == 0) /* clnext detected error */
            {
                return (BLOCKEQ0);
            }
            else if (next_cluster != 0xffffffff)
                cluster = next_cluster;
        } while (next_cluster != 0xffffffff);

        return (pc_cl2sector(pdrive, cluster));
    }
}


DROBJ *pc_fast_mknode(ALIAS_BUFFER *palias, DROBJ *pmom ,byte *filename, byte *fileext, byte attributes, CLUSTERTYPE incluster) /*__fn__*/
{
    DROBJ *pobj;
    BOOLEAN ret_val;
    DOSINODE *pdinodes;
    FINODE lfinode;
    CLUSTERTYPE cluster;
    CLUSTERTYPE cltemp;
    DATESTR crdate;
    BLKBUFF *pbuff;
    DDRIVE *pdrive;
    byte attr;
#if (RTFS_SUBDIRS)
    byte dot_str[4]; /* Make DOT and DOTDOT strings */
    byte null_str[4];
#endif

    ret_val = TRUE;

    if (!pmom || !pmom->pdrive)
    {
        rtfs_set_errno(PEINTERNAL); /* pc_mknode: Internal error*/
        return (0);
    }
    /* Make sure the file/directory name is legal */
    if (!validate_filename(filename, fileext))
    {
        rtfs_set_errno(PEINVALIDPATH);
        return (0);
    }


    pdrive = pmom->pdrive;
    cluster = incluster;
#if (RTFS_SUBDIRS)
    if (attributes & ADIRENT)
    {
        /*Unless renaming a directory we grab a cluster for a new dir and */
        /* clear it   */
        if (!incluster)
        {
            /* pc_alloc_dir will set errno */
            cluster = pc_alloc_dir(pdrive,pmom);
            if (!cluster)
                ret_val = FALSE;
            else if (!pc_clzero( pdrive , cluster ) )
            {
                /* clzero will set errno */
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
                ret_val = FALSE;
            }
        }
    }
#endif

    if (!ret_val)
    {
        return (0);
    }

    /* For a subdirectory. First make it a simple file. We will change the
        attribute after all is clean */
    attr = attributes;
#if (RTFS_SUBDIRS)
    if (attr & ADIRENT)
        attr = ANORMAL;
#endif
    /* Allocate an empty DROBJ and FINODE to hold the new file   */

    pobj = pc_allocobj();
    if (!pobj)
    {
        return (0);
    }
    /* Set up the drive link */
    pobj->pdrive = pmom->pdrive;

    /* Load the inode copy name,ext,attr,cluster, size,datetime  */
    /* Convert pobj to native and stitch it in to mom   */
    if (!pc_fast_insert_inode(palias, pobj , pmom, attr, cluster, filename, fileext))
    {
        if (cluster && !incluster)
            FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
        pc_freeobj(pobj);
        return (0);
    }

#if (RTFS_SUBDIRS)
    /* Now if we are creating subdirectory we have to make the DOT and DOT DOT
        inodes and then change pobj s attribute to ADIRENT
        The DOT and DOTDOT are not buffered inodes. We are simply putting
        the to disk  */
    if ( attributes & ADIRENT)
    {
        if (incluster)
        {
            pbuff = pc_read_blk( pdrive , pc_cl2sector(pdrive , incluster));
            if (!pbuff)
            {
                pc_freeobj(pobj);
                return (0);
            }
        }
        else
        {
            /* Set up a buffer to do surgery   */
            pbuff = pc_init_blk( pdrive , pc_cl2sector(pdrive , cluster));
            if (!pbuff)
            {
                pc_freeobj(pobj);
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
                return (0);
            }
        }
        pdinodes = (DOSINODE *) &pbuff->data[0];
        /* Load DOT and DOTDOT in native form                     */
        /* DOT first. It points to the begining of this sector    */
        dot_str[0] = CS_OP_ASCII('.');dot_str[1]=CS_OP_ASCII('\0');
        null_str[0] = CS_OP_ASCII('\0');

        /* Load the time and date stamp to be used for "." and ".."
           from the subdirectory that we are creating. In this way
           the three directory entries will all have the same timestamp
           value */
        crdate.time = pobj->finode->ftime;
        crdate.date = pobj->finode->fdate;
        pc_init_inode( &lfinode, dot_str,null_str, ADIRENT|ARCHIVE,
                            cluster, /*size*/ 0L , &crdate);
        /* And to the buffer in intel form   */
        pc_ino2dos (pdinodes, &lfinode);
        /* Now DOTDOT points to mom s cluster   */
        cltemp = pc_get_parent_cluster(pdrive, pobj);
        dot_str[0] = CS_OP_ASCII('.');dot_str[1] = CS_OP_ASCII('.');
        dot_str[2] = CS_OP_ASCII('\0');
        null_str[0] = CS_OP_ASCII('\0');
        pc_init_inode( &lfinode, dot_str, null_str, ADIRENT|ARCHIVE, cltemp,
                    /*size*/ 0L , &crdate);
        /* And to the buffer in intel form   */
        pc_ino2dos (++pdinodes, &lfinode );
        /* Write the cluster out   */
        if ( !pc_write_blk ( pbuff ) )
        {
            pc_discard_buf(pbuff);
            pc_freeobj(pobj);
            if (!incluster)
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
            return (0);
        }
        else
            pc_release_buf(pbuff);

        /* And write the node out with the original attributes   */
        pobj->finode->fattribute = (byte)(attributes|ARCHIVE);

        /* Convert to native. Overwrite the existing inode.Set archive/date  */
        if (!pc_update_inode(pobj, TRUE, TRUE))
        {
            pc_freeobj(pobj);
            if (!incluster)
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
            return (0);
        }
    }
#endif
    ret_val = FATOP(pdrive)->fatop_flushfat(pdrive->driveno);

    if (ret_val)
    {
        return (pobj);
    }
    else
    {
        pc_freeobj(pobj);
        return (0);
    }
}

#if (VFAT)
BOOLEAN pc_seglist2disk(DDRIVE * pdrive, SEGDESC *s, byte *lfn);
void pc_zeroseglist(SEGDESC *s);
void pc_addtoseglist(SEGDESC *s, BLOCKT my_block, int my_index);
void pc_reduceseglist(SEGDESC *s);

static BOOLEAN pc_fast_malias(ALIAS_BUFFER *pabuff, byte *fname, byte *fext, byte *input_file, DROBJ *dest);

static BOOLEAN pc_fast_insert_inode(ALIAS_BUFFER *pabuff, DROBJ *pobj , DROBJ *pmom, byte attr, CLUSTERTYPE initcluster, byte *filename, byte *fileext)
{
    BLKBUFF *pbuff;
    DIRBLK *pd;
    DOSINODE *pi;
    int i;
    CLUSTERTYPE cluster;
    DDRIVE *pdrive;
    DATESTR crdate;
    byte vffilename[9],vffileext[4];
    byte cksum;
    BOOLEAN end_of_dir;
    int n_segs;

    cluster = 0;
    /* How many segments do we need   */
    n_segs = (rtfs_cs_strlen(filename) + 12 )/13;

    RTFS_ARGSUSED_PVOID((void *) fileext);
    if (!pc_fast_malias(pabuff, vffilename,vffileext, (byte *)filename,pmom))
        return (FALSE); /* pc_malias set errno */

    pc_init_inode( pobj->finode, vffilename, vffileext,
                    attr, initcluster, /*size*/ 0L ,pc_getsysdate(&crdate) );

    /* Set up pobj      */
    pdrive = pobj->pdrive = pmom->pdrive;
    pobj->isroot = FALSE;
    pd = &pobj->blkinfo;

    /* Now get the start of the dir   */
    pd->my_block = pd->my_frstblock = pc_firstblock(pmom);

    if (!pd->my_block)
    {
        rtfs_set_errno(PEINVALIDBLOCKNUMBER);   /* pc_insert_inode: Internal error, invalid block id */
        return (FALSE);
    }
    else
        pd->my_index = 0;

    /* for pc_fast_insert_inode find the block to scan from. For the root
       directory of FAT12/16 scan from the begining of the directory.
       For subdirectories seek to the first block of the the last cluster
       in the directory chain */
    {
        BLOCKT blockno;
        blockno = pc_fast_l_start_block(pobj->pdrive, pd->my_block);
        if (!blockno)
            return (FALSE); /* Errno set already */
        else
            pd->my_block = blockno;
    }

    /* Read the data   */
    pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    if (!pbuff)
        return(FALSE);
    /* If the alias contains Kanji E5 this goes on disk as 05 so we have to
       checksum it with 05 */
    if (pobj->finode->fname[0] == 0xe5)
    {
        pobj->finode->fname[0] = 0x05;
        cksum = pc_cksum((byte*)pobj->finode);
        pobj->finode->fname[0] = 0xe5;
    }
    else
         cksum = pc_cksum((byte*)pobj->finode);
    end_of_dir = FALSE;
    pc_zeroseglist(&pobj->finode->s);
    pobj->finode->s.ncksum = cksum;
    while (pbuff)
    {
        i = pd->my_index = 0;
        pi = (DOSINODE *) &pbuff->data[0];
        /* look for a slot   */
        while ( i < INOPBLOCK )
        {
            if (pi->fname[0] == CS_OP_ASCII('\0'))
                end_of_dir = TRUE;
            if (pi->fname[0] == PCDELETE || end_of_dir)
            {
                /* Note that we fake an allocation of n_segs + 1 segments.
                   this scheme makes sure that there is room for the DOSINODE
                   immediately after the segment list. We reduce the segment
                   count before we write the segments */
                if (pobj->finode->s.nsegs != (n_segs +1))
                {
                    pc_addtoseglist(&pobj->finode->s, pd->my_block, i);
                }
                /* Write the lfn and dos inode if we have the space   */
                if (pobj->finode->s.nsegs == (n_segs +1))
                {
                    /* Write the long file name entries to their respective
                        locations */
                    /* Drop the segment count by one before we write.
                       with this scheme we fall out of the loop and
                       put the DOSINODE in the correct location */
                    pc_reduceseglist(&pobj->finode->s);
                    if (!pc_seglist2disk(pobj->pdrive, &pobj->finode->s, filename))
                    {
                        pc_discard_buf(pbuff);
                        goto clean_and_fail;
                    }
                    pd->my_index = i;
                    /* Update the DOS disk   */
                    pc_ino2dos( pi, pobj->finode );
                    /* Write the data   */
                    /* Mark the inode in the inode buffer   */
                    if (pc_write_blk(pbuff))
                    {
                        pc_marki(pobj->finode , pobj->pdrive , pd->my_block,
                               pd->my_index );
                        pc_release_buf(pbuff);
                        return(TRUE);
                    }
                    else
                    {
                        pc_release_buf(pbuff);
                        return(FALSE);
                    }
                }
            }
            else
            {
                pc_zeroseglist(&pobj->finode->s);
            }
            i++;
            pi++;
        }
        /* Not in that block. Try again   */
        pc_release_buf(pbuff);
        /* Update the objects block pointer   */
        rtfs_set_errno(0);  /* Should already be 0 but don't chance it */
        if (!pc_next_block(pobj))
        {
            if (get_errno())    /* The zero was an error return */
                return (FALSE);
            /* Ok:There are no slots in mom. We have to make one. And copy our stuff in  */
            cluster = pc_grow_dir(pobj->pdrive, pmom);
            if (!cluster)
                return (FALSE);

            /* Do not forget where the new item is   */
            pd->my_block = pc_cl2sector(pobj->pdrive , cluster);
            pd->my_index = 0;

            /* Zero out the cluster    */
            if (!pc_clzero( pobj->pdrive , cluster ) )
                goto clean_and_fail;

            /* Copy the item into the first block   */
            pobj->pblkbuff = pbuff = pc_init_blk( pobj->pdrive , pd->my_block);
            if (!pbuff)
                goto clean_and_fail;
        }
        else    /* 02-06-2003 - Added Else, had been lost */
            pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    }

clean_and_fail:
    if (cluster)
        pc_truncate_dir(pdrive, pmom, cluster);
    return (FALSE);
}



static BOOLEAN pc_fast_load_alias_buffer(ALIAS_BUFFER *pabuff, DROBJ *pmom)
{
BLOCKT curblock;
DOSINODE *pi;
BLKBUFF *pbuff;
int i;
BOOLEAN end_of_dir;
byte *pfile, *pext;

    /* Start from the beginning of the directory and buffer all non deleted
      8.3 file names */

    /* Now get the start of the dir   */
    curblock = pc_firstblock(pmom);
    if (!curblock)
        return (FALSE);    /* pc_firstblock(pmom); set errno */

    pfile = pabuff->alias_buffer_data;
    pext =  pfile + 8;

    /* Read the data   */
    pbuff = pc_read_blk(pmom->pdrive, curblock);
    if (!pbuff)
        return(FALSE);
    end_of_dir = FALSE;
    while (pbuff)
    {
        pi = (DOSINODE *) &pbuff->data[0];
        /* look for a slot   */
        for(i =0; i < INOPBLOCK;i++,pi++)
        {
            if (pi->fname[0] == CS_OP_ASCII('\0'))
            {
                end_of_dir = TRUE;
                break;
            }
            if (pi->fname[0] != PCDELETE && pi->fattribute != CHICAGO_EXT)
            {
                /* Return filled buffer if it is too small to hold the whole
                   sub-directory */
                if (pabuff->alias_buffer_count >= pabuff->alias_buffer_size)
                    return(TRUE);
                copybuff(pfile, pi->fname, 8);
                copybuff(pext, pi->fext, 3);
                pabuff->alias_buffer_count += 1;
                pfile += 11;
                pext =  pfile + 8;
            }
        }
        pc_release_buf(pbuff);
        pbuff = 0;
        /* Get next block.  */
        if (!end_of_dir)
        {
            rtfs_set_errno(0);  /* Should already be 0 but don't chance it */
            curblock = pc_l_next_block(pmom->pdrive, curblock);
            if (curblock)
                pbuff = pc_read_blk(pmom->pdrive, curblock);
            else
            {
                if (get_errno())    /* The zero was an error return */
                    return (FALSE);
            }
        }
    }
    return (TRUE);
}


static BOOLEAN  alias_cmp(byte *a, byte *b, int n)
{
int i;
    for (i = 0; i < n; i++, a++, b++)
    {
        if (*a != *b)
            return(FALSE);
    }
    return(TRUE);
}

static BOOLEAN pc_fast_malias(ALIAS_BUFFER *pabuff, byte *fname, byte *fext, byte *input_file, DROBJ *dest) /*__fn__*/
{
    int try;
    DROBJ *pobj;
    BOOLEAN aliasunique;
    byte alias[26];
    byte ascii_alias[26];
    byte curr_ascii_alias[26], curr_fname[12],curr_fext[4];


    /* See if already a valid alias. If so we use */
    /* Note: This always fails for unicode */
    /* For Fast, if the filename itself is an alias then no need
       to search since we know it does not already exist */
    if (pc_cs_malias(alias, input_file, -1)) /*__fn__*/
    {
        pc_ascii_str2upper(alias,alias);
        {
            rtfs_set_errno(0);  /* Clear PENOENT */
            pc_ascii_fileparse(fname,fext,(byte *)input_file);
            pc_ascii_str2upper(fname,fname);
            pc_ascii_str2upper(fext,fext);
            return(TRUE);
        }
    }

    /* If an alias buffer is provided and the current count is zero, preload
       the alias buffer with aliases that already exist in the subdirectory
       if the subdirectory is empty the load routine will return immediately */
    if (pabuff && !pabuff->alias_buffer_count && pabuff->alias_buffer_size)
    {
        if (!pc_fast_load_alias_buffer(pabuff, dest))
            return(FALSE);
    }

    /* Loop building up alias names and testing if unique */
    try=0; /* i=0; i is the alias index */
    do
    {
        try++;
        if (!pc_cs_malias(alias, input_file, try))
        {
            /* Ran out of valid alias names. Use PENOSPC as errno */
            rtfs_set_errno(PENOSPC);
            return(FALSE);
        }
        if (pabuff && (pabuff->alias_buffer_count < pabuff->alias_buffer_size))
        {
            /* Parse the alias into 8.3 */
            CS_OP_CS_TO_ASCII_STR(curr_ascii_alias,alias);
            pc_ascii_fileparse(curr_fname,curr_fext,curr_ascii_alias);
            {
                int i;
                byte *pfile, *pext;
                pfile = pabuff->alias_buffer_data;
                pext =  pfile + 8;
                aliasunique = TRUE;

                for (i = 0; i < pabuff->alias_buffer_count; i++)
                {
                    if (alias_cmp(curr_fname, pfile, 8) &&
                        alias_cmp(curr_fext, pext, 3))
                    {
                        aliasunique = FALSE;
                        break;
                    }
                    pfile += 11;
                    pext  += 11;
                }
                if (aliasunique)
                {
                    pabuff->alias_buffer_count += 1;
                    copybuff(pfile, curr_fname, 8);
                    copybuff(pext, curr_fext, 3);
                }
            }
        }
        else
        {
            pobj = pc_get_inode(0,dest,alias,0,GET_INODE_MATCH);
            if(pobj)
            {
                aliasunique = FALSE;
                pc_freeobj(pobj);
            }
            else
            {
                if (get_errno() != PENOENT)
                    return(FALSE);
                aliasunique = TRUE;
            }
        }
    }
    while(!aliasunique);

    rtfs_set_errno(0); /* Clear PENOENT */
    /* Parse the alias into 8.3 */
    CS_OP_CS_TO_ASCII_STR(ascii_alias,alias);
    pc_ascii_fileparse(fname,fext,ascii_alias);

    return(TRUE);
}

#else

BOOLEAN pc_fast_insert_inode(ALIAS_BUFFER *pabuff, DROBJ *pobj , DROBJ *pmom, byte attr, CLUSTERTYPE initcluster, byte *filename, byte *fileext)
{
    BLKBUFF *pbuff;
    DIRBLK *pd;
    DOSINODE *pi;
    int i;
    CLUSTERTYPE cluster;
    DDRIVE *pdrive;
    DATESTR crdate;

    pc_init_inode( pobj->finode, filename, fileext,
                    attr, initcluster, /*size*/ 0L ,pc_getsysdate(&crdate) );

    /* Set up pobj      */
    pdrive = pobj->pdrive = pmom->pdrive;
    pobj->isroot = FALSE;
    pd = &pobj->blkinfo;

    /* Now get the start of the dir   */
    pd->my_block = pd->my_frstblock = pc_firstblock(pmom);

    if (!pd->my_block)
    {
        rtfs_set_errno(PEINVALIDBLOCKNUMBER);   /* pc_insert_inode: Internal error, invalid block id */
        return (FALSE);
    }
    else
        pd->my_index = 0;

    /* for pc_fast_insert_inode find the block to scan from. For the root
       directory of FAT12/16 scan from the begining of the directory.
       For subdirectories seek to the first block of the the last cluster
       in the directory chain */
    {
        BLOCKT blockno;
        blockno = pc_fast_l_start_block(pobj->pdrive, pd->my_block);
        if (!blockno)
            return (FALSE); /* Errno set already */
        else
            pd->my_block = blockno;
    }

    /* Read the data   */
    pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    if (!pbuff)
        return(FALSE);
    while (pbuff)
    {
        i = pd->my_index = 0;
        pi = (DOSINODE *) &pbuff->data[0];
        /* look for a slot   */
        while ( i < INOPBLOCK )
        {
            /* End of dir if name is 0   */
            if ( (pi->fname[0] == CS_OP_ASCII('\0')) || (pi->fname[0] == PCDELETE) )
            {
                pd->my_index = (word)i;
                /* Update the DOS disk   */
                pc_ino2dos( pi, pobj->finode );
                /* Write the data   */
                if (pc_write_blk(pbuff))
                {
                    /* Mark the inode in the inode buffer   */
                    pc_marki(pobj->finode , pobj->pdrive , pd->my_block,
                            pd->my_index );
                    pc_release_buf(pbuff);
                    return(TRUE);
                }
                else
                {
                    pc_discard_buf(pbuff);
                    return(FALSE);
                }
            }
            i++;
            pi++;
        }
        /* Not in that block. Try again   */
        pc_release_buf(pbuff);
        /* Update the objects block pointer   */
        rtfs_set_errno(0);  /* Clear errno to be safe */
        if (!pc_next_block(pobj))
        {
            if (get_errno())
                return(FALSE);
            else
                break;
        }
        pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    }

    cluster = pc_grow_dir(pdrive, pmom);
    if (!cluster)
        return (FALSE);
    /* Do not forget where the new item is   */
    pd->my_block = pc_cl2sector(pobj->pdrive , cluster);
    pd->my_index = 0;

    /* Zero out the cluster    */
    if (!pc_clzero( pobj->pdrive , cluster ) )
        goto clean_and_fail;
    /* Copy the item into the first block   */
    pbuff = pc_init_blk( pobj->pdrive , pd->my_block);
    if (!pbuff)
        goto clean_and_fail;
    pc_ino2dos (  (DOSINODE *) &pbuff->data[0] ,  pobj->finode ) ;
    /* Write it out   */
    if ( !pc_write_blk ( pbuff ) )
    {
        pc_discard_buf(pbuff);
        goto clean_and_fail;
    }

    /* We made a new slot. Mark the inode as belonging there   */
    pc_marki(pobj->finode , pobj->pdrive , pd->my_block, pd->my_index );
    pc_release_buf(pbuff);
    return (TRUE);
clean_and_fail:
    pc_truncate_dir(pdrive, pmom, cluster);
    return (FALSE);
}

#endif

#if (INCLUDE_FAST_TESTCODE)

dword total_n_reads,total_n_writes;

byte * make_file_name(byte *inbuff,int dirno, int fileno)
{
    sprintf((char *) inbuff, "longfile%d%d.tst", dirno, fileno);
    return((byte *) inbuff);
}
BOOLEAN make_file(byte *dirname, int dirno, int fileno)
{
int fd;
byte filebuff[64];
byte pathbuff[256];

    sprintf(pathbuff, "%s\\%s", dirname, (char *) make_file_name(filebuff, dirno, fileno));
    fd = po_open(pathbuff, (PO_BINARY|PO_RDWR|PO_CREAT), (PS_IWRITE | PS_IREAD));
    if (fd < 0)
        return(FALSE);
    if (po_write(fd, pathbuff, 256) != 256)
        return(FALSE);
    po_close(fd);
    return(TRUE);
}


BOOLEAN move_file(ALIAS_BUFFER *pabuff, byte *tdirname, byte *dirname, int dirno, int fileno)
{
byte filebuff[64];
byte frpathbuff[256];
byte topathbuff[256];
    sprintf(frpathbuff, "%s\\%s", dirname, (char *) make_file_name(filebuff, dirno, fileno));
    sprintf(topathbuff, "%s\\%s", tdirname, (char *) make_file_name(filebuff, dirno, fileno));
    if (!pc_fast_mv(pabuff, frpathbuff, topathbuff))
        return(FALSE);
    return(TRUE);
}

char *destdir = "dest";
char *dirnames[4] = {"source1","source2","source3","source4"};

#define NDIRS 4
#define NFILES 200
#define ABUFFSIZE 1000

static byte alias_data[ABUFFSIZE*11];

void test_fast()
{
byte filebuff[64];
int i, j;


    if (!pc_mkdir(destdir))
    {
      printf("Mkdir Failed on %s\n", destdir);
      return;
    }

    for (i = 0; i < NDIRS; i++)
    {
        if (!pc_mkdir(dirnames[i]))
        {
            printf("Mkdir Failed on %s\n", dirnames[i]);
        }
    }
    for (i = 0; i < NDIRS; i++)
    {
        for (j = 0; j < NFILES; j++)
        {
            if (!make_file(dirnames[i],i, j))
            {
                printf("make_file failed on %s\n",
                    (char *) make_file_name(filebuff, i, j) );
                return;
            }
        }
    }
    total_n_reads = total_n_writes = 0;
    {
ALIAS_BUFFER a_buff;

    dword start_tick;
    dword end_tick;
    printf("Start renaming now \n");
        start_tick = rtfs_port_elapsed_zero();
    a_buff.alias_buffer_size  = ABUFFSIZE;
    a_buff.alias_buffer_count = 0;
    a_buff.alias_buffer_data = &alias_data[0];
    for (i = 0; i < NDIRS; i++)
    {
        for (j = 0; j < NFILES; j++)
        {
            if (!move_file(&a_buff, destdir, dirnames[i],i, j))
            {
                printf("make_file failed on %s\n",
                    (char *) make_file_name(filebuff, i, j) );
                return;
            }
        }
    }
    end_tick = rtfs_port_elapsed_zero();
    printf("Elapsed tikcs = %d\n", end_tick - start_tick);
    printf("Nreads, nwrites %d %d\n", total_n_reads,total_n_writes);

    }
}
#endif /* (INCLUDE_FAST_TESTCODE) */
