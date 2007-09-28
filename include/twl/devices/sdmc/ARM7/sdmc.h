
#ifndef __SDMC_H__
#define __SDMC_H__

#include <twl.h>
#include <twl/rtfs.h>
#include <twl/devices/sdmc/ARM7/sdmc_types.h>


#ifdef __cplusplus
extern "C" {
#endif



/*********************************************
 RTFS用ドライバインタフェース
*********************************************/
BOOL sdmcRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
int  sdmcRtfsCtrl( int driveno, int opcode, void* pargs);
BOOL sdmcRtfsAttach( int driveno);
BOOL sdmcCheckMedia( void);


BOOL nandRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
int  nandRtfsCtrl( int driveno, int opcode, void* pargs);
BOOL nandRtfsAttach( int driveno, int partition_no);
BOOL nandCheckMedia( void);


/*********************************************
 基本API
*********************************************/
SDMC_ERR_CODE    sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(void),void (*func2)(void));   /* カードドライバ初期化 */
SDMC_ERR_CODE    sdmcReset( void);                                    /* カードリセット */

SDMC_ERR_CODE    sdmcGetStatus(u16 *status);           /* カードドライバの現在の状態を取得する */
u32              sdmcGetCardSize(void);                /* カード全サイズの取得 */

/*SD I/FのFIFOを使ってリードする（高速）*/
SDMC_ERR_CODE    sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);/* テスト用カードリード */
/*リードする*/
SDMC_ERR_CODE    sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);    /* テスト用カードリード */

/*SD I/FのFIFOを使ってライトする（高速）*/
SDMC_ERR_CODE    sdmcWriteFifo( const void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);/* テスト用カードライト */
/*ライトする*/
SDMC_ERR_CODE    sdmcWrite( const void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);    /* テスト用カードライト */

u16           sdmcSelectedNo(void);
SDMC_ERR_CODE sdmcSelect( u16 select);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__SDMC_H__*/
