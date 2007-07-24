#include <twl_sp.h>

#include <twl/cs/cs.h>
#include <el/elf_loader.h>
#include <devices/sdmc/ARM7/sdmc.h>
#include <fatfs/ARM7/rtfs.h>

#include "sample1.h"
#include "dlltest.h"

#define PRINTDEBUG OS_TPrintf
//  #define PRINTDEBUG( ...)    ((void)0)


#define _DLL_LINK_DYNAMIC_ (1)     // 0:static link, 1:dynamic link
#define _DLL_FROM_FILE_    (0)     // 0:link from RAM, 1:link from file


#define DEBUG_DECLARE   OSTick begin
#define DEBUG_BEGIN()   (begin=OS_GetTick())
#define DEBUG_END(str)  OS_TPrintf( "\n%s was consumed %d msec.\n", #str, (int)OS_TicksToMilliSeconds(OS_GetTick()-begin))



/*---------------------------------------------------------------------------*
    static変数
 *---------------------------------------------------------------------------*/
static u16 path_str[512/sizeof(u16)]; //ロングファイル名
static u32 lib_buf[8192];
static u32 obj_buf[8192];
int        fd;
u32        alloc_total_size = 0;
u32        alloc_max_size = 0;

#if (_DLL_LINK_DYNAMIC_ == 1)
global_func_p   global_func;
g_func_p        g_func;
#endif

/*---------------------------------------------------------------------------*
    static関数
 *---------------------------------------------------------------------------*/
static void display_entries( void);
u32 readlib( u32 offset, void* buf, u32 size);
int no_data;    //DLLが使うstatic側グローバル変数


static void *myAlloc(size_t size)
{
    return( OS_Alloc( size));
}

static void myFree(void *ptr)
{
    OS_Free( ptr);
}


/*
 *  メインタスク
 */
void TwlSpMain( void)
{
    OSHeapHandle   heapHandle;
    ElDesc dll_desc;
    u8     rslt;
    u32    free_bytes, free_blocks, total_blocks;
    u32    i, len, obj_size;
    int    result;
    DEBUG_DECLARE;

    // OS初期化
    OS_Init();

    OS_InitTick();
    OS_InitAlarm();
    
    // PXI初期化、ARM9と同期
    PXI_Init();

    // ヒープ領域設定
//    heapHandle = InitializeAllocateSystem();
  
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    OS_InitThread();
  
  
    // vmsk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)) ;
    PRINTDEBUG( "Sample program starts.\n");

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

    CS_Sjis2Unicode( path_str, "E:");
    if( !rtfs_pc_set_default_drive( (unsigned char*)path_str)) {
        PRINTDEBUG( "pc_set_default_drive (E) failed\n");
        while( 1){};
    }
  

    /*-----DLLファイルオープン  -----*/
//    CS_Sjis2Unicode( path_str, "\\libsampledll_sp.twl.nodbg.a");
//    fd = po_open( (byte*)path_str, (PO_BINARY|PO_WRONLY), PS_IREAD);
    CS_Sjis2Unicode( path_str, "\\LIBSAM~1.a");
    fd = po_open( (byte*)path_str, (PO_BINARY|PO_WRONLY), PS_IREAD);
    if( fd < 0) {
        PRINTDEBUG( "po_open failed.\n");
        while( 1) {};
    }
    PRINTDEBUG( "po_open success.\n");
    /*-------------------------------*/

#if (_DLL_FROM_FILE_ == 1)
#else
    /*----- DLLファイルをRAMへコピー -----*/
    obj_size = po_lseek( fd, 0, PSEEK_END);
    po_lseek( fd, 0, PSEEK_SET);
    if( obj_size > 0) {
        po_read( fd, (void*)obj_buf, obj_size);
    }else{
        PRINTDEBUG( "po_lseek failed.\n");
    }
    /*---------------------------------------*/
#endif
  
    elInit( myAlloc, myFree);
    elInitDesc( &dll_desc);
    
//    elLoadLibraryfromFile( &dll_desc, fd, lib_buf);
    len = po_lseek( fd, 0, PSEEK_END);
    po_lseek( fd, 0, PSEEK_SET);
    DEBUG_BEGIN();
#if (_DLL_FROM_FILE_ == 1)
    elLoadLibrary( &dll_desc, (ElReadImage)readlib, len, lib_buf); //SD上のファイルからリンク
    DEBUG_END(elLoadLibrary from file);
#else
    elLoadLibraryfromMem( &dll_desc, obj_buf, obj_size, lib_buf); //RAMからリンク
    DEBUG_END(elLoadLibrary from RAM);
#endif
    PRINTDEBUG( "alloc size : 0x%x\n", alloc_max_size);

  
    elAddStaticSym();

  
    DEBUG_BEGIN();
    elResolveAllLibrary( &dll_desc);
    DEBUG_END(elResolveAllLibrary);

  
#if (_DLL_LINK_DYNAMIC_ == 1)
    PRINTDEBUG( "LINK : dynamic\n");
    global_func = (global_func_p)elGetGlobalAdr( "global_func\0");
    PRINTDEBUG( "global_func : 0x%x\n", global_func);
    g_func = (g_func_p)elGetGlobalAdr( "g_func\0");
    PRINTDEBUG( "g_func : 0x%x\n", g_func);
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
