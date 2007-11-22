/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/

/***************************************************************************
    fs_test -  Test Failsafe mode

 Description
    Perform a series of tests on ERTFS-Pro to verify correct functioning
    of FailSafe mode.

 Summary:
    BOOLEAN fs_test(byte *path)

 Returns
    Returns TRUE if all test succeeded False otherwise.

 Note: fs_test() sends output to the console through the macro FSDEBUG().
 which is defined:
  #define FSDEBUG(X) printf("%s\n", X);
 To run quietly replace this definition with the following:
  #define FSDEBUG(X)

***************************************************************************/

#include <rtfs.h>

#if (INCLUDE_CS_UNICODE)
byte *testtree[] = {
                    (byte *)L"\\FSTSDIR1",
                    (byte *)L"\\FSTSDIR1\\FSTSDIR2",
                    (byte *)L"\\FSTSDIR1\\FSTSDIR2\\FSTSDIR3",
                    (byte *)L"\\FSTSDIR1\\FSTSDIR2\\FSTSDIR3\\FSTDIR4",
                    0};
byte *testfiles[] = {
                    (byte *)L"\\FSTSFIL1",
                    (byte *)L"\\FSTSFIL2",
                    (byte *)L"\\FSTSFIL3",
                    (byte *)L"\\FSTSFIL4",
                    (byte *)L"\\FSTSFIL5",
                    0};

byte *apitestfile = (byte *)"\\FSTSDIR1\\FSTSFIL1";
byte *apimovedfile = (byte *)"\\FSTSDIR1\\FSTSMOV1";
#else
byte *testtree[] = {
                    (byte *)"\\FSTSDIR1",
                    (byte *)"\\FSTSDIR1\\FSTSDIR2",
                    (byte *)"\\FSTSDIR1\\FSTSDIR2\\FSTSDIR3",
                    (byte *)"\\FSTSDIR1\\FSTSDIR2\\FSTSDIR3\\FSTDIR4",
                    0};
byte *testfiles[] = {
                    (byte *)"\\FSTSFIL1",
                    (byte *)"\\FSTSFIL2",
                    (byte *)"\\FSTSFIL3",
                    (byte *)"\\FSTSFIL4",
                    (byte *)"\\FSTSFIL5",
                    0};

byte *apitestfile = (byte *)"\\FSTSDIR1\\FSTSFIL1";
byte *apimovedfile = (byte *)"\\FSTSDIR1\\FSTSMOV1";
#endif


#if (INCLUDE_FAILSAFE_CODE)
/* #define FSDEBUG(X) printf("%s\n", X); */
#define FSDEBUG(X) RTFS_PRINT_STRING_2(USTRING_SYS_NULL,(byte *)X,PRFLG_NL);

#define DO_FILE_NVIO_TEST 1
#define DO_INDEX_TEST 1
#define DO_API_TEST 1
#define DO_FSAPI_TEST 1
#define DO_RESTORE_TEST 1

BOOLEAN fs_test_nvio_main(byte *path);
BOOLEAN fs_test_indexing_main(byte *path);
BOOLEAN fs_test_api_main(byte *path);
BOOLEAN fs_test_fsapi_main(byte *path);
BOOLEAN fs_test_restore_main(byte *path);
BOOLEAN fs_test_nvio_delete_fsfile(byte *path);

extern int rand(void);

/* Data and structures for test purposes */
byte dummybuff[512];
FAILSAFECONTEXT test_fscontext;
#define TEST_BLOCKMAPSIZE 64
FSBLOCKMAP test_failsafe_blockmap_array[TEST_BLOCKMAPSIZE];

/* private routines imported from prfscore.c for test purposes */
dword fs_map_block(FAILSAFECONTEXT *pfscntxt, dword blockno);
dword fs_block_map_find_replacement(FAILSAFECONTEXT *pfscntxt, dword replacement_block, int *error);
dword fs_block_map_scan(FAILSAFECONTEXT *pfscntxt, dword blockno, int *error);
dword fs_check_mapped(FAILSAFECONTEXT *pfscntxt, dword blockno, int *error);
BOOLEAN pro_failsafe_checksum_index(FAILSAFECONTEXT *pfscntxt, int *error);
BOOLEAN fs_flush_index_pages(FAILSAFECONTEXT *pfscntxt);
int pro_failsafe_flush_header(FAILSAFECONTEXT *pfscntxt, dword status);
dword * fs_map_index_page(FAILSAFECONTEXT *pfscntxt, int index_page, BOOLEAN writing);

/* private routines in this file for test purposes */
BOOLEAN open_index_test(byte *path, int flags, int block_map_size, dword user_journal_size);
BOOLEAN open_index_test_full(byte *path, int flags, int block_map_size, dword user_journal_size);
void fs_test_make_filename(byte *buffer, byte *path, byte *filename);
dword fs_test_fill_file(byte *path, byte *filename, int nblocks);
BOOLEAN fs_test_fill_disk(byte *path, byte *filename);
BOOLEAN fs_test_rm_file(byte *path, byte *filename);
BOOLEAN fs_test_mktree(byte *path);
BOOLEAN fs_test_chkstats(CHKDISK_STATS *p1,CHKDISK_STATS *p2);
static void fs_test_clear_index_buffers(void);
int fs_test_commit_failure(FAILSAFECONTEXT *pfscntxt,BOOLEAN force_checksum_error);

int fs_test(byte *path)
{
#if (DO_INDEX_TEST)
    FSDEBUG("INDEX TEST: Begin")
    if (!fs_test_indexing_main(path))
    {
        FSDEBUG("INDEX TEST: Failed")
        return(0);
    }
    FSDEBUG("INDEX TEST: Success")
#endif
#if (DO_FILE_NVIO_TEST)
    FSDEBUG("NVIO TEST: Begin")
    if (!fs_test_nvio_main(path))
    {
        FSDEBUG("NVIO TEST: Failed")
        return(0);
    }
    FSDEBUG("NVIO TEST: Success")
#endif
#if (DO_API_TEST)
    FSDEBUG("API TEST: Begin")
    if (!fs_test_api_main(path))
    {
        FSDEBUG("API TEST: Failed")
        return(0);
    }
    FSDEBUG("API TEST: Success")
#endif
#if (DO_RESTORE_TEST)
    FSDEBUG("RESTORE TEST: Begin")
    if (!fs_test_restore_main(path))
    {
        FSDEBUG("RESTORE TEST: Fail")
        return(0);
    }
    FSDEBUG("RESTORE TEST: Success")
#endif
#if (DO_FSAPI_TEST)
    FSDEBUG("FSAPI TEST: Begin")
    if (!fs_test_fsapi_main(path))
    {
        FSDEBUG("FSAPI TEST: Failed")
        return(0);
    }
    FSDEBUG("FSAPI TEST: Success")
#endif
    FSDEBUG("FAILSAFE TEST SUITE: Success")
    return(1);
}

#if (DO_INDEX_TEST)
/* #define BIG_TEST_SIZE (4 * (CFG_NUM_INDEX_BUFFERS * 128)) */
#define BIG_TEST_SIZE 4096
dword map_check[BIG_TEST_SIZE];
static BOOLEAN fs_test_map_cache(dword num_to_map);
static BOOLEAN _fs_test_map_cache(dword num_to_map, int fill_op);
static BOOLEAN fs_test_index_errors(void);

BOOLEAN fs_test_indexing_main(byte *path)
{
struct fsblockmap *save_freelist;
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    fs_test_nvio_delete_fsfile(path);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0))
        return(FALSE);
    if (test_fscontext.num_remap_blocks > BIG_TEST_SIZE)
    {
        FSDEBUG("INDEX TEST: BIG_TEST_SIZE too small to run index test. recompile")
        return(FALSE);
    }
    /* Test mapping with all fitting in cache */
    FSDEBUG("INDEX TEST: Test mapping with cache > # journaled blocks")
    if (!fs_test_map_cache(test_fscontext.blockmap_size-1))
        return(FALSE);
    if (!test_fscontext.blockmap_freelist) /* should be one left */
        return(FALSE);
    /* Test with not all fitting in cache */
    FSDEBUG("INDEX TEST: Test mapping with cache < # journaled blocks")
    if ((dword)test_fscontext.blockmap_size >= test_fscontext.num_remap_blocks)
        return(FALSE);
    fs_rewind_failsafe(&test_fscontext);
    if (!fs_test_map_cache(test_fscontext.num_remap_blocks))
        return(FALSE);
    /* It's full make sure another map call fails */
    if (fs_map_block(&test_fscontext, 1023) != 0)
        return(FALSE);
    /* Test with no cache */
    FSDEBUG("INDEX TEST: Test mapping with no cache")
    fs_rewind_failsafe(&test_fscontext);
    save_freelist = test_fscontext.blockmap_freelist;
    test_fscontext.blockmap_freelist = 0;
    if (!fs_test_map_cache(test_fscontext.num_remap_blocks))
        return(FALSE);
    if (test_fscontext.total_blocks_mapped != test_fscontext.num_remap_blocks)
        return(FALSE);
    /* It's full make sure another map call fails */
    FSDEBUG("INDEX TEST: Testing Journal File Full")
    if (fs_map_block(&test_fscontext, 1023) != 0)
        return(FALSE);
    if (get_errno() != PEJOURNALFULL)
        return(FALSE);
    fs_rewind_failsafe(&test_fscontext);
    test_fscontext.blockmap_freelist = save_freelist;

    /* Now do the test simulating a very large file */
    FSDEBUG("INDEX TEST: Test mapping with cache and very large journal")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE,BIG_TEST_SIZE))
        return(FALSE);
    if (!fs_test_map_cache(test_fscontext.num_remap_blocks))
        return(FALSE);
    return(TRUE);
}

static BOOLEAN fs_test_map_cache(dword num_to_map)
{
    /* Test with ascending  descending, then random fill */
    if (!_fs_test_map_cache(num_to_map, 1))
        return(FALSE);
    if (!_fs_test_map_cache(num_to_map, -1))
        return(FALSE);
    if (!_fs_test_map_cache(num_to_map, 0))
        return(FALSE);
    if (!fs_test_index_errors())
        return(FALSE);
    return(TRUE);
}
static BOOLEAN fs_test_index_errors(void)
{
int error,rval;
dword *pdw;
    /* Now see if the checksum is bad */
    if (pro_failsafe_flush_header(&test_fscontext, FS_STATUS_PROCESSING))
    { /* First flush the index header to the buffer and to disk */
        return(FALSE);
    }
    if (!pro_failsafe_checksum_index(&test_fscontext, &error))
        return(FALSE);
    /* Now clear index pages and checksum (from journal, not cache)*/
    fs_test_clear_index_buffers();
    if (!pro_failsafe_checksum_index(&test_fscontext, &error))
        return(FALSE);
    /* Should be in MUST restore state */
    fs_test_clear_index_buffers();
    rval = failsafe_restore_internal(&test_fscontext, FALSE, FALSE);
    if (rval != FS_STATUS_MUST_RESTORE)
        return(FALSE);
    /* Now Simulate Index header problems and check restore */
    /* Force a checksum error */
    FSDEBUG("INDEX TEST: Testing Index Checksum Error")
    pdw = fs_map_index_page(&test_fscontext, 0, TRUE);
    if (!pdw) return(FALSE);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_INDEX_CHECKSUM),test_fscontext.journal_checksum-1);
    if (!fs_flush_index_pages(&test_fscontext))  return(FALSE);
    fs_test_clear_index_buffers();
    rval = failsafe_restore_internal(&test_fscontext, FALSE,FALSE);
    if (rval != FS_STATUS_BAD_CHECKSUM)
        return(FALSE);
    fs_test_clear_index_buffers();
    pdw = fs_map_index_page(&test_fscontext, 0, TRUE);
    if (!pdw) return(FALSE);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_INDEX_CHECKSUM),test_fscontext.journal_checksum);
    if (!fs_flush_index_pages(&test_fscontext))  return(FALSE);

    /* Force an out of date error */
    FSDEBUG("INDEX TEST: Testing Index Out of Date Error")
    pdw = fs_map_index_page(&test_fscontext, 0, TRUE);
    if (!pdw) return(FALSE);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_FREE_CLUSTERS),test_fscontext.pdrive->known_free_clusters-1);
    if (!fs_flush_index_pages(&test_fscontext))  return(FALSE);
    fs_test_clear_index_buffers();
    rval = failsafe_restore_internal(&test_fscontext, FALSE,FALSE);
    if (rval != FS_STATUS_OUT_OF_DATE) return(FALSE);
    fs_test_clear_index_buffers();
    pdw = fs_map_index_page(&test_fscontext, 0, TRUE);
    if (!pdw) return(FALSE);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_FREE_CLUSTERS),test_fscontext.pdrive->known_free_clusters);
    if (!fs_flush_index_pages(&test_fscontext))  return(FALSE);

    /* Force a bad journal error */
    FSDEBUG("INDEX TEST: Testing Journal File Signature Error")
    pdw = fs_map_index_page(&test_fscontext, 0, TRUE);
    if (!pdw) return(FALSE);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_VERSION    ) ,FS_JOURNAL_VERSION+2);
    if (!fs_flush_index_pages(&test_fscontext))  return(FALSE);
    fs_test_clear_index_buffers();
    rval = failsafe_restore_internal(&test_fscontext, FALSE,FALSE);
    if (rval != FS_STATUS_BAD_JOURNAL) return(FALSE);
    fs_test_clear_index_buffers();
    pdw = fs_map_index_page(&test_fscontext, 0, TRUE);
    if (!pdw) return(FALSE);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_VERSION    ) ,FS_JOURNAL_VERSION);
    if (!fs_flush_index_pages(&test_fscontext))  return(FALSE);
    fs_test_clear_index_buffers();
    FSDEBUG("INDEX TEST: Testing Good Journal File")
    if (pro_failsafe_flush_header(&test_fscontext, FS_STATUS_COMPLETE))
        return(FALSE);
    fs_test_clear_index_buffers();
    rval = failsafe_restore_internal(&test_fscontext, FALSE,FALSE);
    if (rval != FS_STATUS_OK) return(FALSE);
    return(TRUE);
}


static void fs_test_clear_index_buffers(void)
{
int i;
    for (i = 0; i < CFG_NUM_INDEX_BUFFERS; i++)
        test_fscontext.index_offset[i] = 0;
}

static BOOLEAN _fs_test_map_cache(dword num_to_map, int fill_op)
{
int error;
struct fsblockmap *pbm;
dword i, mapped_block, fake_block, ltemp, num_mapped;

   fs_rewind_failsafe(&test_fscontext);
   for (i = 0; i < BIG_TEST_SIZE; i++) map_check[i] = 0;

   if (fill_op == -1)
   { /* reverse order */
    FSDEBUG("INDEX TEST: Filling Journal File in descending order")
    i = num_to_map;
    ltemp = 0;
    while(i--)/* Repeat the first test backward */
    {
        fake_block = 1024+i;
        mapped_block = fs_map_block(&test_fscontext, fake_block);
        if (mapped_block != ltemp+test_fscontext.num_index_blocks)
            return(FALSE);
        map_check[i] = mapped_block;
        ltemp++;
    }
   }
    else if (fill_op == 0) /* random */
    {
        FSDEBUG("INDEX TEST: Filling Journal File in random order")
        num_mapped = 0;
        while (num_mapped < num_to_map)
        {
            fake_block = 1024+(rand()%num_to_map);
            mapped_block = fs_check_mapped(&test_fscontext, fake_block, &error);
            if (error)
                return(FALSE);
            if (!mapped_block)
            {
                mapped_block = fs_map_block(&test_fscontext, fake_block);
                if (mapped_block != num_mapped+test_fscontext.num_index_blocks)
                    return(FALSE);
                map_check[fake_block-1024] = mapped_block;
                num_mapped += 1;
            }
        }
    }
    else /* ascending order */
    {
        FSDEBUG("INDEX TEST: Filling Journal File in ascending order")
        fake_block = 1024;
        for (ltemp = 0; ltemp < num_to_map; ltemp++, fake_block++)
        { /* Remap a bunch of blocks */
            mapped_block = fs_map_block(&test_fscontext, fake_block);
            if (mapped_block != ltemp+test_fscontext.num_index_blocks)
                return(FALSE);
            map_check[ltemp] = mapped_block;
        }
    }
    fake_block = 1024;
    for (ltemp = 0; ltemp < num_to_map; ltemp++, fake_block++)
    {  /* Verify that they are all there */
        mapped_block = fs_map_block(&test_fscontext, fake_block);
        if (map_check[ltemp] != mapped_block)
            return(FALSE);
    }
    /* Fail if this pass added any new blocks to the map */
    if (test_fscontext.total_blocks_mapped != num_to_map)
        return(FALSE);

    FSDEBUG("INDEX TEST: Testing Journal Block Search")
    fake_block = 1024;
    for (ltemp = 0; ltemp < num_to_map; ltemp++, fake_block++)
    {   /* Check the cache layer and then scan the block layer */
        mapped_block = fs_check_mapped(&test_fscontext, fake_block, &error);
        if (error || (map_check[ltemp] != mapped_block))
            return(FALSE);
        mapped_block = fs_block_map_scan(&test_fscontext, fake_block, &error);
        if (error || (map_check[ltemp] != mapped_block))
            return(FALSE);
    }
    for (ltemp = 0; ltemp < num_to_map; ltemp++)
    { /* Now do the reverse. given the replacement give back the block */
        mapped_block = fs_block_map_find_replacement(&test_fscontext,
            map_check[ltemp], &error);
        if (error || mapped_block != ltemp+1024)
            return(FALSE);
    }
    ltemp = num_to_map;
    while(ltemp--)/* Repeat the first test backward */
    {
        fake_block = 1024+ltemp;
        mapped_block = fs_map_block(&test_fscontext, fake_block);
        if (map_check[ltemp] != mapped_block)
            return(FALSE);
    }
    if (test_fscontext.total_blocks_mapped != num_to_map)
        return(FALSE);

    /* Make sure the sorted map is correct */
    FSDEBUG("INDEX TEST: Testing Sorted Journal Block List")
    if (test_fscontext.blockmap_freelist)
    {
        pbm = test_fscontext.sorted_blockmap;
        fake_block = 1024;
        for (i = 0; i < test_fscontext.total_blocks_mapped; i++, fake_block++)
        {
            if (!pbm)
                return(FALSE);
            if (pbm->blockno != fake_block)
                return(FALSE);
            pbm = pbm->pnext_by_block;
        }
        if (pbm)
            return(FALSE);
    }
    return(TRUE);
}


#endif

#if (DO_FILE_NVIO_TEST)
dword fs_test_nvio_make_failsafe_file(byte *path, int fsfsize, BOOLEAN clean);

BOOLEAN fs_test_nvio_main(byte *path)
{
int step;
dword blockno, fsblockno;
int fsfsize;
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!fs_test_nvio_delete_fsfile(path))
    {
        step = 1;
test_failed:
        return(FALSE);
    }
    fsfsize = 100;
    /* First verify placement of the journal file */
    FSDEBUG("NVIO TEST: verify journal file placement")
    blockno = fs_test_fill_file(path, (byte *) testfiles[0], fsfsize);
    if (!blockno) {step = 2; goto test_failed;}
    fs_test_rm_file(path, (byte *) testfiles[0]);
    fsblockno = fs_test_nvio_make_failsafe_file(path, fsfsize, TRUE);
    if (blockno!=fsblockno) {step = 3; goto test_failed;}
    /* Now verify placement of the journal file with fragments */
    FSDEBUG("NVIO TEST: verify journal file placement with fragmentation")
    blockno = fs_test_fill_file(path, (byte *) testfiles[0], fsfsize/2);
    if (!blockno) {step = 3; goto test_failed;}
    blockno = fs_test_fill_file(path, (byte *) testfiles[1], fsfsize/2);
    if (!blockno) {step = 4; goto test_failed;}
    blockno = fs_test_fill_file(path, (byte *) testfiles[2], fsfsize);
    if (!blockno) {step = 5; goto test_failed;}
    fs_test_rm_file(path, (byte *) testfiles[0]); /* Create the hole */
    fs_test_rm_file(path, (byte *) testfiles[2]); /* failsafe file should go here */
    fsblockno = fs_test_nvio_make_failsafe_file(path, fsfsize, TRUE);
    fs_test_rm_file(path, (byte *) testfiles[1]); /* release the fragment */
    if (blockno!=fsblockno) {step = 6; goto test_failed;}
    /* Now verify that we can change the Failsafe file size */
    FSDEBUG("NVIO TEST: verify changing journal file size")
    fsblockno = fs_test_nvio_make_failsafe_file(path, fsfsize/2, FALSE);
    if (!fsblockno) {step = 7; goto test_failed;}
    blockno = fs_test_fill_file(path, (byte *) testfiles[0], fsfsize);
    if (!blockno) {step = 8; goto test_failed;}
    blockno = fs_test_fill_file(path, (byte *) testfiles[1], fsfsize);
    if (!blockno) {step = 9; goto test_failed;}
    fs_test_rm_file(path, (byte *) testfiles[1]); /* new failsafe file should go here */
    fsblockno = fs_test_nvio_make_failsafe_file(path, fsfsize, TRUE);
    fs_test_rm_file(path, (byte *) testfiles[0]); /* release the fragment */
    if (blockno!=fsblockno) {step = 10; goto test_failed;}
{
dword blocks_total, blocks_free;
    if (pc_free(path, &blocks_total, &blocks_free) < 0)
    {step = 10; goto test_failed;}
    if (blocks_free > 200000) /* 100 mbytes */
    {
        FSDEBUG("NVIO TEST: not testing file create error with full disk")
        ;
    }
    else
    {
        /* Now verify the behavior of Failsafe when the disk is full */
        FSDEBUG("NVIO TEST: test journal file create error with full disk")
        if (!fs_test_fill_disk(path, (byte *) testfiles[0]))
        {step = 10; goto test_failed;}
        fsblockno = fs_test_nvio_make_failsafe_file(path, fsfsize, TRUE);
        step = 0;
        if (fsblockno) step = 11;
        if (get_errno() != PEFSCREATE) step = 12;
        if (step) goto test_failed;
        pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
        fs_test_rm_file(path, (byte *) testfiles[0]); /* un-fill the disk */
    }
}
    if (!fs_test_nvio_delete_fsfile(path))
         return(FALSE);
     return(TRUE);
}

dword fs_test_nvio_make_failsafe_file(byte *path, int fsfsize, BOOLEAN clean)
{
byte buffer[32];
dword ret_val;
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.configuration_flags = FS_MODE_AUTORECOVER;
    test_fscontext.blockmap_size = 0;
    test_fscontext.user_journal_size = fsfsize;
    if (!pro_failsafe_init(path, &test_fscontext))
        return(0);
    if (!pc_pwd(path, buffer)) /* Mount the disk */
    {
        pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
        return(0);
    }
    ret_val = test_fscontext.nv_buffer_handle;
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (clean)
        fs_test_nvio_delete_fsfile(path);
    return(ret_val);
}


#endif

#if (DO_RESTORE_TEST)



BOOLEAN fs_test_restore_main(byte *path)
{
CHKDISK_STATS chkstat[4];
byte buffer[32];
int rval;
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    fs_test_nvio_delete_fsfile(path);

    /* Shut down failsafe, save check disk info */
    if (!open_index_test(path, 0, TEST_BLOCKMAPSIZE, 0)) return(FALSE);
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(FALSE);

    /* Check manual restore mode */
    /* Make a directory, get check disk info */
    FSDEBUG("RESTORE TEST: test manual restore")
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,FALSE)) return(FALSE);
    /* Checkdisk and compare to the original check disk, should be the same */
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(FALSE);
    /* Verify restore */
    rval = pro_failsafe_restore(path,&test_fscontext,FALSE,FALSE);
    if (rval != FS_STATUS_MUST_RESTORE) return(FALSE);
    rval = pro_failsafe_restore(path,&test_fscontext,TRUE,FALSE);
    if (rval != FS_STATUS_RESTORED) return(FALSE);
    /* Checkdisk,compare to check disk with mkdir when we were journaling */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(FALSE);

    if (!open_index_test(path, 0, TEST_BLOCKMAPSIZE, 0)) return(FALSE);
    /* Check manual restore with checksum error */
    FSDEBUG("RESTORE TEST: test manual restore with bad Journal file")
    if (!pc_rmdir(testtree[0])) return(FALSE);
    /* Checkdisk and compare to the original check disk, should be the same */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(FALSE);
    /* commit but simulate a checksum error */
    if (fs_test_commit_failure(&test_fscontext,TRUE)) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Verify restore */
    rval = pro_failsafe_restore(path,&test_fscontext,FALSE,FALSE);
    if (rval != FS_STATUS_BAD_CHECKSUM) return(FALSE);
    rval = pro_failsafe_restore(path,&test_fscontext,TRUE,FALSE);
    if (rval != FS_STATUS_BAD_CHECKSUM) return(FALSE);

    /* Check auto restore mode with a bad journal file */
    /* Check the clear function of pro_failsafe_restore */
    FSDEBUG("RESTORE TEST: test auto restore with bad Journal file")
    if (open_index_test(path, FS_MODE_AUTORESTORE, TEST_BLOCKMAPSIZE, 0)) return(FALSE);
    if (get_errno() != PEFSRESTOREERROR) return(FALSE);
    /* Clear the error and see if we can now mount */
    FSDEBUG("RESTORE TEST: Test pro_failsafe_restore() clear function")
    if (pro_failsafe_restore(path,0,FALSE,TRUE)!=FS_STATUS_OK) return(FALSE);
    if (!pc_pwd(path, buffer)) /* Mount the disk */
        return(FALSE);
    /* The directory should be there now remove and commit */
    if (!pc_rmdir(testtree[0])) return(FALSE);
    if (!pro_failsafe_commit(path)) return(FALSE);
    /* Checkdisk and compare to the original check disk, should be the same */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(FALSE);

    /* Create a directory but force a journal error */
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,TRUE)) return(FALSE);
    /* Automount should fail */
    if (pc_pwd(path, buffer))  return(FALSE);
    if (get_errno() != PEFSRESTOREERROR) return(FALSE);
    /* Now switch to autorevover mode, should work */
    FSDEBUG("RESTORE TEST: auto recover with bad Journal file")
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTORESTORE, TEST_BLOCKMAPSIZE, 0)) return(FALSE);
    /* Checkdisk and compare to the original check disk, should be the same */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(FALSE);

    /* Create a directory force journal commit interruption */
    FSDEBUG("RESTORE TEST: test auto restore with good Journal file")
    if (!pc_mkdir(testtree[0])) return(FALSE);
    /* Checkdisk,compare to check disk with mkdir when we were journaling */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,FALSE)) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Checkdisk and compare to the original check disk, should be the same */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(FALSE);
    if (!open_index_test(path, FS_MODE_AUTORESTORE, TEST_BLOCKMAPSIZE, 0)) return(FALSE);
    /* Checkdisk,compare to check disk with mkdir when we were journaling */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(FALSE);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(FALSE);
    return(TRUE);
}
#endif /* (DO_RESTORE_TEST) */


#if (DO_API_TEST)

/* API Calls to test: */

int fs_test_api_mkdir(byte *path);
int fs_test_api_mv(byte *path);
int fs_test_api_deltree(byte *path);
int fs_test_api_rmdir(byte *path);
int fs_test_api_open(byte *path);
int fs_test_api_unlink(byte *path);
int fs_test_api_change(byte *path,int test);


BOOLEAN fs_test_api_main(byte *path)
{
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    fs_test_nvio_delete_fsfile(path);
    FSDEBUG("API TEST")
    if (!pc_set_default_drive(path))
        return(FALSE);
    /* Make sure failsafe file exists */
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
 /* pc_mkdir */
    FSDEBUG("API TEST: pc_mkdir")
    if (fs_test_api_mkdir(path)) return(FALSE);
/* pc_rmdir */
    FSDEBUG("API TEST: pc_rmdir")
    if (fs_test_api_rmdir(path)) return(FALSE);
/* po_open */
    FSDEBUG("API TEST: po_open")
    if (fs_test_api_open(path)) return(FALSE);
/* pc_unlink */
    FSDEBUG("API TEST: pc_unlink")
    if (fs_test_api_unlink(path)) return(FALSE);
/* pc_mv */
    FSDEBUG("API TEST: pc_mv")
    if (fs_test_api_mv(path)) return(FALSE);
/* pc_deltree */
    FSDEBUG("API TEST: pc_deltree")
    if (fs_test_api_deltree(path)) return(FALSE);
/* pc_set_attributes() */
    FSDEBUG("API TEST: pc_set_attributes")
    if (fs_test_api_change(path, 0)) return(FALSE);
/* po_extend_file() */
    FSDEBUG("API TEST: po_extend_file")
    if (fs_test_api_change(path, 1)) return(FALSE);
/* po_chsize() */
    FSDEBUG("API TEST: po_chsize")
    if (fs_test_api_change(path, 2)) return(FALSE);
/* po_truncate() */
    FSDEBUG("API TEST: po_truncate")
    if (fs_test_api_change(path, 3)) return(FALSE);
/* po_write() */
    FSDEBUG("API TEST: po_write")
    if (fs_test_api_change(path, 4)) return(FALSE);
/* po_close() */
    FSDEBUG("API TEST: po_close")
    if (fs_test_api_change(path, 5)) return(FALSE);
    FSDEBUG("API TEST: Success")
    return(TRUE);
}

int fs_test_api_mkdir(byte *path)
{
CHKDISK_STATS chkstat[4];
    FSDEBUG("API TEST:            test journalling")
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_mkdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(1);

    /* With no failsafe */
    if (!pc_mkdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);

    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!pc_rmdir(testtree[0])) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if (!pc_mkdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!pc_rmdir(testtree[0])) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_mkdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pc_rmdir(testtree[0])) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (pc_mkdir(testtree[0])) return(1);
    if (get_errno() != PEJOURNALFULL) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    return(0);
}

int fs_test_api_rmdir(byte *path)
{
CHKDISK_STATS chkstat[4];
    if (!pc_mkdir(testtree[0])) return(1);

    FSDEBUG("API TEST:            test journalling")
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_rmdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(1);

    /* With no failsafe */
    if (!pc_rmdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pc_mkdir(testtree[0])) return(1);

    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if (!pc_rmdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pc_mkdir(testtree[0])) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_rmdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pc_mkdir(testtree[0])) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (pc_rmdir(testtree[0])) return(1);
    if (get_errno() != PEJOURNALFULL) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(1);
    return(0);
}
static void fs_test_freefile(int fd)
{
PC_FILE *pfile;
DROBJ *pobj;
    pfile = prtfs_cfg->mem_file_pool+fd;
    pobj = pfile->pobj;
    pfile->is_free = TRUE;
    if (pobj)
       pc_freeobj(pobj);
}

int fs_test_api_open(byte *path)
{
int fd;
CHKDISK_STATS chkstat[4];
    if (!pc_mkdir(testtree[0])) return(1);

    FSDEBUG("API TEST:            test journalling")
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
    fs_test_freefile(fd);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(1);

    /* With no failsafe */
    if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
    fs_test_freefile(fd);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);

    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!pc_unlink(apitestfile)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
    fs_test_freefile(fd);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!pc_unlink(apitestfile)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
    fs_test_freefile(fd);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pc_unlink(apitestfile)) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD))) >= 0) return(1);
    if (get_errno() != PEJOURNALFULL) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */

    if (!pc_rmdir(testtree[0])) return(1);
    return(0);
}

int fs_test_create_file(byte *fname,int fsize)
{
int fd;
    if ((fd = po_open(fname,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_EXCL),(word)(PS_IWRITE | PS_IREAD))) < 0)
        return(1);
    if (fsize)
        if (po_write(fd,&dummybuff[0], fsize) != fsize) return(1);
    po_close(fd);
    return(0);
}

int fs_test_api_unlink(byte *path)
{
CHKDISK_STATS chkstat[4];
    if (!pc_mkdir(testtree[0])) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);

    FSDEBUG("API TEST:            test journalling")
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(1);

    /* With no failsafe */
    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);

    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (pc_unlink(apitestfile)) return(1);
    if (get_errno() != PEJOURNALFULL) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_rmdir(testtree[0])) return(1);
    return(0);
}
BOOLEAN fs_test_check_mv(BOOLEAN shouldbemoved)
{
ERTFS_STAT stat;
    if (shouldbemoved)
    {
        if (pc_stat(apimovedfile, &stat) != -1)
             if (pc_stat(apitestfile, &stat) == -1)
                return(TRUE);
    }
    else
    {
        if (pc_stat(apitestfile, &stat) != -1)
            if (pc_stat(apimovedfile, &stat) == -1)
            return(TRUE);
    }
    return(FALSE);
}

int fs_test_api_mv(byte *path)
{
    if (!pc_mkdir(testtree[0])) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);

    FSDEBUG("API TEST:            test journalling")
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_mv(apitestfile, apimovedfile)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (!fs_test_check_mv(FALSE)) return(1);

    /* With no failsafe */
    if (!pc_mv(apitestfile, apimovedfile)) return(1);
    if (!fs_test_check_mv(TRUE)) return(1);
    if (!pc_mv(apimovedfile,apitestfile)) return(1);
    if (!fs_test_check_mv(FALSE)) return(1);

    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if (!pc_mv(apitestfile, apimovedfile)) return(1);
    if (!fs_test_check_mv(TRUE)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!fs_test_check_mv(TRUE)) return(1);
    if (!pc_mv(apimovedfile,apitestfile)) return(1);
    if (!fs_test_check_mv(FALSE)) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_mv(apitestfile, apimovedfile)) return(1);
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!fs_test_check_mv(TRUE)) return(1);
    if (!pc_mv(apimovedfile,apitestfile)) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (pc_mv(apitestfile, apimovedfile)) return(1);
    if (get_errno() != PEJOURNALFULL) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */

    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_rmdir(testtree[0])) return(1);
    return(0);
}

int fs_test_api_deltree(byte *path)
{
CHKDISK_STATS chkstat[4];

    FSDEBUG("API TEST:            test journalling")
    if (!fs_test_mktree(path)) return(1);
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_deltree(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[2])) return(1);

    /* With no failsafe */
    if (!pc_deltree(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!fs_test_mktree(path)) return(1);

    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if (!pc_deltree(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!fs_test_mktree(path)) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (!pc_deltree(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[2], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[1],&chkstat[2])) return(1);
    if (!fs_test_mktree(path)) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (pc_deltree(testtree[0])) return(1);
    if (get_errno() != PEJOURNALFULL) return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_deltree(testtree[0])) return(1);
    return(0);
}


int fs_test_change_size(int fd, int test)
{
    if (test == 1)
    {
    dword fileextend, new_size;
        fileextend = (dword) 32*1024;
        fileextend += 1023;
        if (!po_extend_file(fd, fileextend, &new_size, 0, PC_FIRST_FIT) ||
             new_size != fileextend)
             return(1);
    }
    else if (test == 2)
    {
    dword filesize;
         filesize = (dword) 32*1024;
         filesize = filesize * 2;
         filesize += 1023;
        if (po_chsize(fd, (long)filesize)) return (1);
    }
    else if (test == 3)
    {
        if (!po_truncate(fd, 0)) return(1);
    }
    else if (test == 4)
    {
        if (po_write(fd,&dummybuff[0], 32*1024) != 32*1024) return(1);
        if (po_write(fd,&dummybuff[0], 32*1024) != 32*1024) return(1);
        if (po_write(fd,&dummybuff[0], 1023) != 1023) return(1);
    }
    return(0);
}
int fs_test_check_size(int test, BOOLEAN changed)
{
dword filesize;
ERTFS_STAT stat;
    if (changed)
    {
        filesize = (dword) 32*1024;
        filesize = filesize * 2;
        filesize += 1023;
        if (test == 3)
            filesize = 0;
    }
    else
        filesize = (dword) 32*1024;
   if (pc_stat(apitestfile, &stat) != 0) return(1);
   if (stat.st_size != filesize) return(1);
   return(0);
}

/* 0 pc_set_attributes() */
/* 1 po_extend_file() */
/* 2 po_chsize() */
/* 3 po_truncate() */
/* 4 po_write() */
/* 5 po_close() */
/* Use either pc_set_attributes or po_close to change attributes */
BOOLEAN fs_set_attributes(int test, byte *pfilename, byte attribute)
{
int fd;
PC_FILE *pfile;
    if (test == 0)
    {
        if (!pc_set_attributes(pfilename, attribute)) return(FALSE);
    }
    else if (test == 5)
    {
        fd = po_open(pfilename,(word)(PO_BINARY|PO_RDWR),(word)(PS_IWRITE | PS_IREAD));
        if (fd < 0)
            return(FALSE);
        pfile = prtfs_cfg->mem_file_pool+fd;
        pfile->pobj->finode->fattribute = attribute;
        pfile->needs_flush = TRUE;
        if (po_close(fd) != 0)
            return(FALSE);
    }
    return(TRUE);
}


int fs_test_api_change(byte *path,int test)
{
CHKDISK_STATS chkstat[4];
byte attributes[2];
int fd;
BOOLEAN attrib_test;

    if (test == 0 || test == 5)
        attrib_test = TRUE;
    else
        attrib_test = FALSE;

    fd = -1;
    FSDEBUG("API TEST:            test journalling")
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(1);
    if (!pc_mkdir(testtree[0])) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);
    if (attrib_test)
        if (!pc_get_attributes(apitestfile, &attributes[0])) return(1);
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (attrib_test)
    {
        if (!fs_set_attributes(test, apitestfile, (byte)(attributes[0]|ASYSTEM))) return(1);
        if (!pc_get_attributes(apitestfile, &attributes[1])) return(1);
        if (attributes[1] != ((byte)(attributes[0]|ASYSTEM))) return(1);
    }
    else
    {
        if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
        if (fs_test_change_size(fd, test)) return(1);
        if (fs_test_check_size(test,TRUE)) return(1);
        po_close(fd);
    }
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    /* Failsafe was aborted. See that no disk changes were permanent */
    if (attrib_test)
    {
        if (!pc_get_attributes(apitestfile, &attributes[1])) return(1);
        if (attributes[1] != attributes[0]) return(1);
    }
    else
        if (fs_test_check_size(test,FALSE)) return(1);

    /* With no failsafe */
    if (attrib_test)
    {
        if (!fs_set_attributes(test, apitestfile, (byte)(attributes[0]|ASYSTEM))) return(1);
        if (!pc_get_attributes(apitestfile, &attributes[1])) return(1);
        if (attributes[1] != ((byte)(attributes[0]|ASYSTEM) )) return(1);
        if (!fs_set_attributes(test, apitestfile, attributes[0])) return(1);
    }
    else
    {
        if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
        if (fs_test_change_size(fd, test)) return(1);
        if (fs_test_check_size(test,TRUE)) return(1);
        po_close(fd);
        if (!pc_unlink(apitestfile)) return(1);
        if (fs_test_create_file(apitestfile,32*1024)) return(1);
    }
    /* Check auto commit mode */
    FSDEBUG("API TEST:            test autocommit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT, TEST_BLOCKMAPSIZE, 0)) return(4);
    if (attrib_test)
    {
        if (!fs_set_attributes(test, apitestfile, (byte)(attributes[0]|ASYSTEM))) return(1);
    }
    else
    {
        if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
        if (fs_test_change_size(fd, test)) return(1);
        po_close(fd);
    }
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (attrib_test)
    {
        if (!pc_get_attributes(apitestfile, &attributes[1])) return(1);
        if (attributes[1] != (byte)(attributes[0]|ASYSTEM)) return(1);
    }
    else if (fs_test_check_size(test,TRUE)) return(1);
    if (!pc_unlink(apitestfile)) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);

    /* Check manual commit mode */
    FSDEBUG("API TEST:            test manual commit")
    if (!open_index_test(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (attrib_test)
    {
        if (!fs_set_attributes(test, apitestfile, (byte)(attributes[0]|ASYSTEM))) return(1);
    }
    else
    {
        if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
        if (fs_test_change_size(fd, test)) return(1);
        po_close(fd);
    }
    if (!pro_failsafe_commit(path))  return(1);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (attrib_test)
    {
        if (!pc_get_attributes(apitestfile, &attributes[1])) return(1);
        if (attributes[1] != (byte)(attributes[0]|ASYSTEM)) return(1);
        if (!fs_set_attributes(test, apitestfile, attributes[0])) return(1);
    }
    else if (fs_test_check_size(test,TRUE)) return(1);

    /* Make sure journal full error check works */
    FSDEBUG("API TEST:            test with full Journal")
    if (!pc_unlink(apitestfile)) return(1);
    if (fs_test_create_file(apitestfile,32*1024)) return(1);
    if (!open_index_test_full(path, FS_MODE_AUTORECOVER, TEST_BLOCKMAPSIZE, 0)) return(1);
    if (attrib_test)
    {
        if (fs_set_attributes(test, apitestfile, (byte)(attributes[0]|ASYSTEM))) return(1);
    }
    else
    {
        if ((fd = po_open(apitestfile,(word)(PO_BINARY|PO_RDWR),(word)(PS_IWRITE | PS_IREAD))) < 0) return(1);
        if (fs_test_change_size(fd, test) == 0) return(1);
    }
    if (get_errno() != PEJOURNALFULL) return(1);
    if (fd >= 0)
        po_close(fd);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */

    if (!pc_unlink(apitestfile)) return(1);
    if (!pc_rmdir(testtree[0])) return(1);
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(1);
    if (!fs_test_chkstats(&chkstat[0],&chkstat[1])) return(1);
    return(0);
}


#endif /* (DO_API_TEST) */
#if (DO_FSAPI_TEST)

BOOLEAN fs_test_fsapi_failsafe_init(byte *path);
BOOLEAN fs_test_fsapi_failsafe_commit(byte *path);
BOOLEAN fs_test_fsapi_failsafe_restore(byte *path);

BOOLEAN fs_test_fsapi_main(byte *path)
{
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    fs_test_nvio_delete_fsfile(path);
    if (!pc_set_default_drive(path))
        return(FALSE);
    FSDEBUG("FSAPI TEST: pro_failsafe_init")
    if (!fs_test_fsapi_failsafe_init(path))
    {
        FSDEBUG("FSAPI TEST: pro_failsafe_init Failed")
        return(FALSE);
    }
    FSDEBUG("FSAPI TEST: pro_failsafe_init Success")
    FSDEBUG("FSAPI TEST: pro_failsafe_commit")
    if (!fs_test_fsapi_failsafe_commit(path))
    {
        FSDEBUG("FSAPI TEST: pro_failsafe_commit Failed")
        return(FALSE);
    }
    FSDEBUG("FSAPI TEST: pro_failsafe_commit Success")
    FSDEBUG("FSAPI TEST: pro_failsafe_restore")
    if (!fs_test_fsapi_failsafe_restore(path))
    {
        FSDEBUG("FSAPI TEST: pro_failsafe_restore Failed")
        return(FALSE);
    }
    FSDEBUG("FSAPI TEST: pro_failsafe_restore Success")


    return(TRUE);
}

BOOLEAN fs_test_fsapi_failsafe_init(byte *path)
{
byte buffer[32];
dword ltemp,default_size;
CHKDISK_STATS chkstat[4];
struct fsblockmap *pbm;
int i;
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    fs_test_nvio_delete_fsfile(path);

    FSDEBUG("FSAPI TEST: pro_failsafe_init test mount behavior")
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (test_fscontext.pdrive->mount_valid) return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* Mount the disk */
    if (pro_failsafe_init(path, &test_fscontext))  return(FALSE); /*re-init */
    if (get_errno() != PEFSREINIT) return(FALSE);

    FSDEBUG("FSAPI TEST: pro_failsafe_init test bad inputs")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    /* Test bad flags */
    test_fscontext.configuration_flags =
            (FS_MODE_AUTORESTORE|FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT);
    test_fscontext.configuration_flags = ~test_fscontext.configuration_flags;
    if (pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (get_errno() != PEINVALIDPARMS) return(FALSE);
    /* Test blockmap cache arguments */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.blockmap_size = 1000;
    test_fscontext.blockmap_freelist = 0;
    if (pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (get_errno() != PEINVALIDPARMS)
        return(FALSE);
    /* Test user file size arguments */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.user_journal_size = 1; /* Too small */
    if (pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (get_errno() != PEINVALIDPARMS)
        return(FALSE);
    FSDEBUG("FSAPI TEST: pro_failsafe_init user_journal_size > default")
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.user_journal_size = 0; /* Default */
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* Mount the disk */
    default_size = test_fscontext.num_index_blocks+test_fscontext.num_remap_blocks;
    ltemp = default_size +
            (dword)(test_fscontext.pdrive->secpalloc*2); /* 2 clusters bigger*/
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[0], 0, 0, 0)) return(FALSE); /* Remember disk info */
     rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.user_journal_size = ltemp;
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* Mount the disk */
    ltemp = test_fscontext.num_index_blocks+test_fscontext.num_remap_blocks;
    if (ltemp != test_fscontext.user_journal_size)  return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(FALSE);
    if ((chkstat[1].n_hidden_clusters-chkstat[0].n_hidden_clusters)!=2)
        return(FALSE);
    if ((chkstat[0].n_free_clusters-chkstat[1].n_free_clusters)!=2)
        return(FALSE);
    FSDEBUG("FSAPI TEST: pro_failsafe_init user_journal_size < default")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    ltemp = default_size - (dword)(test_fscontext.pdrive->secpalloc*2); /* 2 clusters smaller*/
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.user_journal_size = ltemp;
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* Mount the disk */
    ltemp = test_fscontext.num_index_blocks+test_fscontext.num_remap_blocks;
    if (ltemp != test_fscontext.user_journal_size)  return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_check_disk(path, &chkstat[1], 0, 0, 0)) return(FALSE);
    if ((chkstat[0].n_hidden_clusters-chkstat[1].n_hidden_clusters)!=2)
        return(FALSE);
    if ((chkstat[1].n_free_clusters-chkstat[0].n_free_clusters)!=2)
        return(FALSE);
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    FSDEBUG("FSAPI TEST: pro_failsafe_init test block map arguments")
    rtfs_memset((void *) &test_failsafe_blockmap_array[0], 0, sizeof(&test_failsafe_blockmap_array));
    test_fscontext.blockmap_size = TEST_BLOCKMAPSIZE;
    test_fscontext.blockmap_freelist = &test_failsafe_blockmap_array[0];
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    pbm = test_fscontext.blockmap_freelist;
    for (i = 0; i < TEST_BLOCKMAPSIZE; i++)
    {  if (!pbm) return(FALSE); pbm = pbm->pnext;}
    if (pbm) return(FALSE);

    FSDEBUG("FSAPI TEST: pro_failsafe_init test FS_MODE_AUTOCOMMIT")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.configuration_flags = FS_MODE_AUTOCOMMIT;
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_mkdir(testtree[0])) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(FALSE);

    FSDEBUG("FSAPI TEST: pro_failsafe_init test FS_MODE_AUTORESTORE")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,FALSE)) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (pc_pwd(path, buffer)) return(FALSE); /* re-mount disk should fail */
    if (get_errno() != PEFSRESTORENEEDED) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (pc_rmdir(testtree[0])) return(FALSE); /* Shouldn't be there */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.configuration_flags = FS_MODE_AUTORESTORE;
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk and autorestore */
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(FALSE); /* Should now be there */

    FSDEBUG("FSAPI TEST: pro_failsafe_init test FS_MODE_AUTORECOVER")
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,TRUE)) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (pc_pwd(path, buffer)) return(FALSE); /* re-mount disk should fail */
    if (get_errno() != PEFSRESTOREERROR) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (pc_rmdir(testtree[0])) return(FALSE); /* Shouldn't be there */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (pc_pwd(path, buffer)) return(FALSE); /* re-mount disk should fail */
    if (get_errno() != PEFSRESTOREERROR) return(FALSE);

    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
      rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.configuration_flags = FS_MODE_AUTORESTORE;
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (pc_pwd(path, buffer)) return(FALSE); /* re-mount disk should fail */
    if (get_errno() != PEFSRESTOREERROR) return(FALSE);

    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
      rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.configuration_flags = FS_MODE_AUTORESTORE|FS_MODE_AUTORECOVER;
    if (!pro_failsafe_init(path, &test_fscontext))  return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk should work */
    return(TRUE);
}

BOOLEAN fs_test_fsapi_failsafe_commit(byte *path)
{
byte buffer[32];
dword ltemp;

    FSDEBUG("FSAPI TEST: pro_failsafe_commit test journal IO error handling")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk */
    ltemp = test_fscontext.nv_buffer_handle; /* Save */
    test_fscontext.nv_buffer_handle = 0;     /* Force errors during journal */
    if (pc_mkdir(testtree[0])) return(FALSE);
    if ( (get_errno() != PEIOERRORWRITEJOURNAL) ||
         (get_errno() != PEIOERRORWRITEJOURNAL))
         return(FALSE);
    test_fscontext.nv_buffer_handle = ltemp;  /* Undo error and try again */
    if (!pc_mkdir(testtree[0])) return(FALSE);
    test_fscontext.nv_buffer_handle = 0;     /* Force errors during commit */
    if (pro_failsafe_commit(path)) return(FALSE);
    if ( (get_errno() != PEIOERRORWRITEJOURNAL) ||
         (get_errno() != PEIOERRORWRITEJOURNAL))
         return(FALSE);
    test_fscontext.nv_buffer_handle = ltemp;  /* Undo error and try again */
    if (!pro_failsafe_commit(path)) return(FALSE);
     pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(FALSE); /* Should be on disk */

    FSDEBUG("FSAPI TEST: pro_failsafe_commit test error handling")

    if (pro_failsafe_commit(path)) return(FALSE); /* Commit without init */
    if (get_errno() != PENOINIT) return(FALSE);

    FSDEBUG("FSAPI TEST: pro_failsafe_commit test commit process")
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk */
    if (!pc_mkdir(testtree[0])) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (pc_rmdir(testtree[0])) return(FALSE); /* should not be there */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk */
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (!pro_failsafe_commit(path)) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(FALSE); /* should be there */
    return(TRUE);
}

BOOLEAN fs_test_fsapi_failsafe_restore(byte *path)
{
byte buffer[32];
int rval;


    FSDEBUG("FSAPI TEST: pro_failsafe_restore test normal restore")
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,FALSE)) return(FALSE);
    rval = pro_failsafe_restore(path,0,TRUE,FALSE);
    if (rval != FS_STATUS_RESTORED) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    if (!pc_rmdir(testtree[0])) return(FALSE);

    FSDEBUG("FSAPI TEST: pro_failsafe_restore test journal data error handling")
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk */
    if (!pc_mkdir(testtree[0])) return(FALSE);
    if (fs_test_commit_failure(&test_fscontext,TRUE)) return(FALSE);
    rval = pro_failsafe_restore(path,0,TRUE,FALSE);
    if (rval != FS_STATUS_BAD_CHECKSUM) return(FALSE);
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */

    FSDEBUG("FSAPI TEST: pro_failsafe_restore test clear function")
    rval = pro_failsafe_restore(path,&test_fscontext,FALSE,TRUE);
    if (rval != FS_STATUS_OK) return(FALSE);
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    if (!pro_failsafe_init(path, &test_fscontext)) return(FALSE);
    if (!pc_pwd(path, buffer)) return(FALSE); /* re-mount disk */
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    return(TRUE);
}
#endif /* (DO_FSAPI_TEST) */

BOOLEAN fs_test_chkstats(CHKDISK_STATS *p1,CHKDISK_STATS *p2)
{
    if (p1->n_user_files != p2->n_user_files)
        return(FALSE);
    if (p1->n_hidden_files != p2->n_hidden_files)
        return(FALSE);
    if (p1->n_user_directories != p2->n_user_directories)
        return(FALSE);
    if (p1->n_free_clusters != p2->n_free_clusters)
        return(FALSE);
    if (p1->n_bad_clusters != p2->n_bad_clusters)
        return(FALSE);
    if (p1->n_file_clusters != p2->n_file_clusters)
        return(FALSE);
    if (p1->n_hidden_clusters != p2->n_hidden_clusters)
        return(FALSE);
    if (p1->n_dir_clusters != p2->n_dir_clusters)
        return(FALSE);
    if (p1->n_crossed_points != p2->n_crossed_points)
        return(FALSE);
    if (p1->n_lost_chains != p2->n_lost_chains)
        return(FALSE);
    if (p1->n_lost_clusters != p2->n_lost_clusters)
        return(FALSE);
    if (p1->n_bad_lfns != p2->n_bad_lfns)
        return(FALSE);
    return(TRUE);
}
void fs_test_make_filename(byte *buffer, byte *path, byte *filename)
{
    rtfs_cs_strcpy(buffer, path);
    rtfs_cs_strcat(buffer, filename);
}

/* Create a contiguous file of nblocks */
dword fs_test_fill_file(byte *path, byte *filename, int nblocks)
{
byte fullname[64];
int fd;
dword needed_size, returned_size;
FILESEGINFO seginfo;

    fs_test_make_filename(fullname, path, filename);
    needed_size = (dword) nblocks*512;
    if ((fd = po_open(fullname,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
    {
        if (!po_extend_file(fd, needed_size, &returned_size, 0, PC_FIRST_FIT) ||
            returned_size != needed_size)
        {
return_error:
            po_close(fd);
            fs_test_rm_file(path, filename);
            return(0);
        }
/* if (pc_get_file_extents(fd, 1, &seginfo, FALSE) != 1)
   Use raw block numbers since we are comparing with the partition base
*/
        if (pc_get_file_extents(fd, 1, &seginfo, TRUE) != 1)
            goto return_error;
        po_close(fd);
        return(seginfo.block);
    }
   return(0);
}

/* Create a contiguous file of nblocks */
BOOLEAN fs_test_fill_disk(byte *path, byte *filename)
{
byte fullname[64];
int fd;
int needed_size,returned_size;

   needed_size = pc_cluster_size(path);
   if (!needed_size)
       return(FALSE);
    fs_test_make_filename(fullname, path, filename);
    if ((fd = po_open(fullname,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
    {
        do
        {
            returned_size = po_write(fd, 0, needed_size);
        } while (returned_size == needed_size);
        po_close(fd);
        if (returned_size == -1)
            return(FALSE);
        else
            return(TRUE);
    }
    return(FALSE);
}

BOOLEAN fs_test_rm_file(byte *path, byte *filename)
{
byte fullname[64];

    fs_test_make_filename(fullname, path, filename);
    if (!pc_unlink(fullname))
        return(FALSE);
    return(TRUE);
}

BOOLEAN fs_test_mktree(byte *path)
{
int i,j,k,fd;
byte dnamebuffer[256];
byte fnamebuffer[256];
ERTFS_STAT stat;
int filesize;

    if (pc_stat(testtree[0], &stat) != -1)
        if (!pc_deltree(testtree[0]))
            return(FALSE);
    filesize =  pc_cluster_size(path) * 4;

    for (i = 0; testtree[i]; i++)
    {
        fs_test_make_filename(dnamebuffer, path, testtree[i]);
        if (!pc_mkdir(dnamebuffer))
            return(FALSE);
        for (j = 0; testfiles[j]; j++)
        {
            fs_test_make_filename(fnamebuffer,dnamebuffer, testfiles[j]);
            if ((fd = po_open(fnamebuffer,(word)(PO_BINARY|PO_RDWR|PO_CREAT|PO_TRUNC),(word)(PS_IWRITE | PS_IREAD))) >= 0)
            {
                if (filesize)
                {
                    k = filesize;
                    /* Write non block alligned bytes */
                    if (j == 0)
                    {
                        while (k < filesize)
                        {
                            if (po_write(fd,dummybuff,100) != 100)
                                goto return_error;
                            k += 100;
                        }
                    }
                    else if (j == 1)
                    { if (po_write(fd,&dummybuff[0],filesize) != filesize)
                                goto return_error;
                    }
                    else if (po_write(fd,0,filesize) != filesize) /* Extend by writing with NULL */
                    {
return_error:
                       po_close(fd);
                       return(FALSE);
                    }
                    po_close(fd);
                }
            }
            else
                return(FALSE);
        }
    }
    return(TRUE);
}

BOOLEAN open_index_test_full(byte *path, int flags, int block_map_size, dword user_journal_size)
{
    if (!open_index_test(path, flags, block_map_size, user_journal_size))
        return(FALSE);
    test_fscontext.total_blocks_mapped =
        test_fscontext.num_remap_blocks;
    return(TRUE);
}


BOOLEAN open_index_test(byte *path, int flags, int block_map_size, dword user_journal_size)
{
byte buffer[32];
    pro_failsafe_shutdown(path, TRUE); /* Shut off failsafe */
    rtfs_memset((void *) &test_fscontext, 0, sizeof(test_fscontext));
    test_fscontext.configuration_flags = flags;
    test_fscontext.blockmap_size = block_map_size;
    test_fscontext.user_journal_size = user_journal_size;
    if (block_map_size)
        test_fscontext.blockmap_freelist = &test_failsafe_blockmap_array[0];
    if (!pro_failsafe_init(path, &test_fscontext))
        return(FALSE);
    if (!pc_pwd(path, buffer)) /* Mount the disk */
    {
        return(FALSE);
    }
    return(TRUE);
}
BOOLEAN fs_test_nvio_delete_fsfile(byte *path)
{
byte filename[64];
byte attrib;
    fs_test_make_filename(filename, path,rtfs_strtab_user_string(USTRING_SYS_FSFILENAME));
    if (!pc_get_attributes(filename, &attrib))
    {
        if (get_errno() == PENOENT)
            return(TRUE);
        else
            return(FALSE);
    }
    if (!pc_set_attributes(filename, 0))
        return(FALSE);
    if (!pc_unlink(filename))
        return(FALSE);
    return(TRUE);
}

int fs_test_commit_failure(FAILSAFECONTEXT *pfscntxt,BOOLEAN force_checksum_error)
{
int   ret_val;


    if (!pfscntxt->total_blocks_mapped)
        return(0);
    if (force_checksum_error)
        pfscntxt->journal_checksum -= 1;
    /* update index information to disk */
    if (pro_failsafe_flush_header(pfscntxt, FS_STATUS_PROCESSING))
    {
       ret_val = -1;
       goto ex_it;
    }
    ret_val = 0;
    pc_dskfree(pfscntxt->pdrive->driveno);
ex_it:
    return (ret_val);
}


#endif /* INCLUDE_FAILSAFE_CODE */
