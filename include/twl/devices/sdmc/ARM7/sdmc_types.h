
#ifndef __SDMC_TYPES_H__
#define __SDMC_TYPES_H__

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif



/*********************************************
 �|�[�g�ԍ�
*********************************************/
typedef enum {
    SDMC_PORT_CARD    = 0x400,
    SDMC_PORT_NAND    = 0x401
}SDMC_PORT_NO;


/*********************************************
 DMA�ԍ�
*********************************************/
typedef enum {
    SDMC_USE_DMA_4    = 0,
    SDMC_USE_DMA_5    = 1,
    SDMC_USE_DMA_6    = 2,
    SDMC_USE_DMA_7    = 3,
    SDMC_NOUSE_DMA    = 0xFF
}SDMC_DMA_NO;


/*********************************************
 �J�[�h�G���[�R�[�h�i�J�[�h�G���[�X�e�[�^�X�ݒ�l�j�A�v���P�[�V�����ŗL��SDCARD_ErrStatus�ɑ΂���
*********************************************/
typedef enum {
    SDMC_NORMAL             =   0,      /* ����I�� */
    SDMC_ERR_COMMAND        =   0x0001, /* CMD�G���[ */
    SDMC_ERR_CRC            =   0x0002, /* CRC�G���[ */
    SDMC_ERR_END            =   0x0004, /* ���s�G���[ */
    SDMC_ERR_TIMEOUT        =   0x0008, /* �R�}���h�^�C���A�E�g */
    SDMC_ERR_FIFO_OVF       =   0x0010, /* FIFO �I�[�o�[�t���[�G���[(INFO2��Illegal write access to buffer) */
    SDMC_ERR_FIFO_UDF       =   0x0020, /* FIFO �A���_�[�t���[�G���[(INFO2��Illegal read access to buffer) */
    SDMC_ERR_WP             =   0x0040, /* WriteProtect�ɂ�鏑�����݃G���[ */
    SDMC_ERR_FPGA_TIMEOUT   =   0x0100, /* FPGA �A�N�Z�X�^�C���A�E�g */
    SDMC_ERR_PARAM          =   0x0200, /* �R�}���h�p�����[�^�G���[ */
    SDMC_ERR_R1_STATUS      =   0x0800, /* Normal response command �J�[�h�X�e�[�^�X �G���[ */
    SDMC_ERR_NUM_WR_SECTORS =   0x1000, /* �������݊����Z�N�^�� �G���[ */
    SDMC_ERR_RESET          =   0x2000, /* �������J�[�h���Z�b�g�R�}���h��1.5�b�^�C���A�E�g�G���[ */
    SDMC_ERR_ILA            =   0x4000, /* �C���[�K���A�N�Z�X�G���[ */
    SDMC_ERR_INFO_DETECT    =   0x8000  /* �J�[�h�r�o�����ʃG���[�r�b�g(IO3) */
}SDMC_ERR_CODE;


/*********************************************
 SD�h���C�o�������ʒʒm���\����
*********************************************/
typedef struct {
    u16    b_flags;                     /* �������e         */
    u16    result;                      /* ���s����         */
    u32    resid;                       /* �ǂ�(����)�T�C�Y */
} SdmcResultInfo;





/*********************************************
 SD�|�[�g��ԕۑ��p�\����
*********************************************/
typedef struct
{
    u16           SD_CID[8];    /* CID�ۑ��p (Card IDentification register) : ID*/
    u16           SD_CSD[8];    /* CSD�ۑ��p (Card Specific Data register) : spec*/
    u16           SD_OCR[2];    /* OCR�ۑ��p (Operation Condition Register) : voltage and status*/
    u16           SD_SCR[4];    /* SCR�ۑ��p (Sd card Configulation Register) : bus-width, card-ver, etc*/
    u16           SD_RCA;       /* RCA�ۑ��p (Relative Card Address register) : address*/
    s16           MMCFlag;
    s16           SDHCFlag;
    s16           SDFlag;
    SDMC_ERR_CODE ErrStatus; /* SDCARD_ErrStatus */
    u32           Status;    /* SDCARD_Status */
    u16           SD_CLK_CTRL_VALUE;
    u16           SD_OPTION_VALUE;

    s16           OutFlag;
    u16           port_no;
}
SDPortContext;





/*********************************************
 SD�X�y�b�N�\����
*********************************************/
typedef struct {
    u32     csd_ver2_flag;              //CSD�t�H�[�}�b�g�o�[�W����(SDHC�̂Ƃ���1)
    u32     memory_capacity;            //data area�̃T�C�Y(512Byte�P��)
    u32     protected_capacity;         //protected area�̃T�C�Y(512Byte�P��)
    u32     card_capacity;              //�J�[�h�S�̂̃T�C�Y(512Byte�P��)

    u32     adjusted_memory_capacity;   //memory_capacity���V�����_(heads*secptrack)�̔{���ɒ��������T�C�Y(cylinders*heads*secptrack�ɂȂ�)
      
    u16     heads;
    u16     secptrack;
    u16     cylinders;
    u16     SC;                         //sectors per cluster
    u16     BU;
    u16     RDE;                        //number of root dir entries(512 fix)
    u32     SS;                         //sector size(512 fix)
    u32     RSC;                        //reserved sector count(1 fix)
//    u32     TS;                         //total sectors
    u16     FATBITS;                    //16 or 32
    u16     SF;                         //sectors per FAT
    u32     SSA;                        //sectors in system area
    u32     NOM;                        //sectors in master boot record
} SdmcSpec;


/*FAT�p�����[�^ TODO:SdmcSpec�𓝍����邱��*/
typedef struct {
    u32    device_capacity;          //�f�o�C�X�S�̂̃T�C�Y(512Byte�P��)
    u32    adjusted_device_capacity;

    u32    memory_capacity;          //data area�̃T�C�Y(512Byte�P��)
    u32    adjusted_memory_capacity; //memory_capacity���V�����_(heads*secptrack)�̔{���ɒ��������T�C�Y(cylinders*heads*secptrack�ɂȂ�)
  
    u16    volume_cylinders;
    
    u16    heads;
    u16    secptrack;
    u16    cylinders;
    u16    SC;                       //sectors per cluster
    u16    BU;
    u16    RDE;                      //number of root dir entries(512 fix)
    u32    SS;                       //sector size(512 fix)
    u32    RSC;                      //reserved sector count(1 fix)
    u16    FATBITS;                  //16 or 32
    u16    SF;                       //sectors per FAT
    u32    SSA;                      //sectors in system area
    u32    NOM;                      //sectors in master boot record

    u32    begin_sect;
} FATSpec;


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__SDMC_TYPES_H__*/