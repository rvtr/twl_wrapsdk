
#ifndef __SDMC_H__
#define __SDMC_H__

#include <twl.h>
#include <twl/rtfs.h>
#include <twl/devices/sdmc/ARM7/sdmc_types.h>


#ifdef __cplusplus
extern "C" {
#endif



/*********************************************
 RTFS�p�h���C�o�C���^�t�F�[�X
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
 ��{API
*********************************************/
SDMC_ERR_CODE    sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(void),void (*func2)(void));   /* �J�[�h�h���C�o������ */
SDMC_ERR_CODE    sdmcReset( void);                                    /* �J�[�h���Z�b�g */

SDMC_ERR_CODE    sdmcGetStatus(u16 *status);           /* �J�[�h�h���C�o�̌��݂̏�Ԃ��擾���� */
u32              sdmcGetCardSize(void);                /* �J�[�h�S�T�C�Y�̎擾 */

/*SD I/F��FIFO���g���ă��[�h����i�����j*/
SDMC_ERR_CODE    sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);/* �e�X�g�p�J�[�h���[�h */
/*���[�h����*/
SDMC_ERR_CODE    sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);    /* �e�X�g�p�J�[�h���[�h */

/*SD I/F��FIFO���g���ă��C�g����i�����j*/
SDMC_ERR_CODE    sdmcWriteFifo( const void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);/* �e�X�g�p�J�[�h���C�g */
/*���C�g����*/
SDMC_ERR_CODE    sdmcWrite( const void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);    /* �e�X�g�p�J�[�h���C�g */

u16           sdmcSelectedNo(void);
SDMC_ERR_CODE sdmcSelect( u16 select);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__SDMC_H__*/
