#include <twl_sp.h>

#include <el/elf_loader.h>
#include <devices/sdmc/ARM7/sdmc.h>
#include <fatfs/ARM7/rtfs.h>

#include "sample1.h"
#include "dlltest.h"

  #define PRINTDEBUG( ...)    ((void)0)


#define _DLL_LINK_DYNAMIC_ (1)


/*---------------------------------------------------------------------------*
    static変数
 *---------------------------------------------------------------------------*/
static u32	lib_buf[8192];
int				fd;
u32         alloc_total_size = 0;
u32         alloc_max_size = 0;

#if (_DLL_LINK_DYNAMIC_ == 1)
global_func_p	global_func;
g_func_p			g_func;
#endif

/*---------------------------------------------------------------------------*
    static関数
 *---------------------------------------------------------------------------*/
static void display_entries( void);
u32 readlib( u32 offset, void* buf, u32 size);

int no_data;

//
//ストリームAPI用関数 (FILE構造体を使いたいなら必要)
//実際にはmalloc/freeなどが使えるようにするためのものなので、
//間違って使ったらサイズが足りなくなるかも
//
/*
#define MY_HEAP_SIZE    (8192*6)//65536
static u8 myHeap[MY_HEAP_SIZE/sizeof(u8)] ATTRIBUTE_ALIGN(32);
static ID myMplID;

static void myInitHeap(void)
{
    T_CMPL cmpl = { TA_TFIFO, MY_HEAP_SIZE, NULL };
    myMplID = acre_mpl(&cmpl);
}
static void *myAlloc(size_t size)
{
    void *ptr;
    ER ercd = pget_mpl(myMplID, size, &ptr);
    if (ercd == E_OK) {
T_RMPL rmpl;
ref_mpl(myMplID, &rmpl);
        alloc_total_size += size;
        if( alloc_total_size > alloc_max_size) {
            alloc_max_size = alloc_total_size;
        }
        return ptr;
    }
    return NULL;
}
static void myFree(void *ptr)
{
    rel_mpl(myMplID, ptr);
}
*/

static void *myAlloc(size_t size)
{
    return( OS_Alloc( size));
}
static void myFree(void *ptr)
{
    OS_Free( ptr);
}


int main(void)
{
  extern void _uitron_start(void);
  _uitron_start();
  return 0;
}

/*
 *  メインタスク
 */
void TwlSpMain( void)
{
    ElDesc dll_desc;
    u32    i;
    u32    free_bytes, free_blocks, total_blocks;
    u8     rslt;
    u32    len;
    int    result;

    // vmsk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)) ;
    PRINTDEBUG( "Sample program starts (exinf = %d).\n", (INT) exinf);
    PRINTDEBUG( "printf test 0.03 = %f\n", 0.03);
    

    /**/
    PRINTDEBUG("Sample program starts.\n");
    {
      OSHeapHandle hh;
      OS_SetSubPrivArenaLo( OS_InitAlloc( OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), OS_GetSubPrivArenaHi(), 1));
      hh = OS_CreateHeap( OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), OS_GetSubPrivArenaHi());
      OS_SetCurrentHeap( OS_ARENA_MAIN_SUBPRIV, hh);
      if( rtfs_init() == FALSE) {
        PRINTDEBUG( "rtfs_init failed.\n");
      }else{
        PRINTDEBUG( "rtfs_init success.\n");
      }
    }
    OS_Alloc( 32);
  

    /*SDドライバ初期化*/
    result = sdmcInit( SDMC_NOUSE_DMA, NULL, NULL);
    if( result != SDMC_NORMAL) {
        PRINTDEBUG( "sdmcInit : failed\n");
        while( 1) {};
    }else{
        PRINTDEBUG( "sdmcInit : success\n");
    }


    /*デバイスドライバの登録*/
    if( sdmcRtfsAttach( 4) == FALSE) {  //sdmcをEドライブにする
        PRINTDEBUG( "sdmcRtfsAttach failed.\n");
    }else{
        PRINTDEBUG( "sdmcRtfsAttach success.\n");
    }

    if( !rtfs_pc_set_default_drive( (unsigned char*)"E:")) {
        PRINTDEBUG( "pc_set_default_drive (E) failed\n");
        while( 1){};
    }
  

    /*----------*/
    fd = po_open( (byte*)"\\libdlltest.a", (PO_CREAT|PO_BINARY|PO_WRONLY), PS_IREAD);
    if( fd < 0) {
        PRINTDEBUG( "po_open failed.\n");
        while( 1) {};
    }
    PRINTDEBUG( "po_open success.\n");
    /*----------*/
  
    elInit( myAlloc, myFree);//OS_Alloc, OS_Free);
    PRINTDEBUG( "%s, %d\n", __FUNCTION__, __LINE__);
    
    elInitDesc( &dll_desc);
    PRINTDEBUG( "%s, %d\n", __FUNCTION__, __LINE__);
    
//    elLoadLibraryfromFile( &dll_desc, fd, lib_buf);
    len = po_lseek( fd, 0, PSEEK_END);
    po_lseek( fd, 0, PSEEK_SET);
    elLoadLibrary( &dll_desc, (ElReadImage)readlib, len, lib_buf);
    PRINTDEBUG( "%s, %d\n", __FUNCTION__, __LINE__);
    PRINTDEBUG( "alloc size : 0x%x\n", alloc_max_size);

    elAddStaticSym();
    PRINTDEBUG( "%s, %d\n", __FUNCTION__, __LINE__);
    
    elResolveAllLibrary( &dll_desc);
    PRINTDEBUG( "%s, %d\n", __FUNCTION__, __LINE__);

#if (_DLL_LINK_DYNAMIC_ == 1)
    PRINTDEBUG( "LINK : dynamic\n");
    global_func = (global_func_p)elGetGlobalAdr( "global_func\0");
    PRINTDEBUG( "0x%x\n", global_func);
    g_func = (g_func_p)elGetGlobalAdr( "g_func\0");
    PRINTDEBUG( "0x%x\n", g_func);
#endif
    
    PRINTDEBUG( "----- dll-func1 execution -----\n");
#if (_DLL_LINK_DYNAMIC_ == 1)
    (*global_func)();
#else
    global_func();
#endif
    
    PRINTDEBUG( "----- dll-func2 execution -----\n");
#if (_DLL_LINK_DYNAMIC_ == 1)
    (*g_func)();
#else
    g_func();
#endif
    
    PRINTDEBUG( "----- dll execution end -----\n");
    

    PRINTDEBUG( "Sample program ends.\n");
//  kernel_exit();
    while( 1 ) {};
}

u32 readlib( u32 offset, void* buf, u32 size)
{
    po_lseek( fd, offset, PSEEK_SET);
    return( po_read( fd, buf, size));
}
