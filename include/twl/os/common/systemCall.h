/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     systemCall.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_SYSTEMCALL_H_
#define TWL_OS_SYSTEMCALL_H_

#define SVC_SHA1_BLOCK_SIZE     64
#define SVC_SHA1_DIGEST_SIZE    20

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u32* head;
    u32* tail;
    u32  size;
}
SVCSignHeapContext;

typedef struct SVCSHA1Context
{
    u32 h0,h1,h2,h3,h4;
    u32 Nl,Nh;
    u32 data[16];
    int num;
    void (*sha_block)(struct SVCSHA1Context *c, const u8 *W, int num);
}
SVCSHA1Context;

typedef struct SVCHMACSHA1Context
{
    SVCSHA1Context  sha1_ctx;
    u8              key[SVC_SHA1_BLOCK_SIZE];
    u32             keylen;
}
SVCHMACSHA1Context;

typedef struct
{
    void*       output;
    const void* input;
    const void* key;
}
SVCSignBuffers;


void SVC_InitSignHeap(
                    SVCSignHeapContext* acmemory_pool,
                    void*           heap,
                    unsigned int    length
                    );

int SVC_DecryptRSA(
                    const SVCSignHeapContext*     acmemory_pool,
                    const SVCSignBuffers*     pData,
                    unsigned int*   len        // �o�̓T�C�Y
                    );

int SVC_DecryptSign(
                    const SVCSignHeapContext*     acmemory_pool,
                    void*           buffer,     //  �o�͗̈�
                    const void*     sgn_ptr,    //  �f�[�^�ւ̃|�C���^
                    const void*     key_ptr     //  �L�[�ւ̃|�C���^
                    );

int SVC_DecryptSignDER(
                    const SVCSignHeapContext*     acmemory_pool,
                    void*           buffer,     //  �o�͗̈�
                    const void*     sgn_ptr,    //  �f�[�^�ւ̃|�C���^
                    const void*     key_ptr     //  �L�[�ւ̃|�C���^
                    );

void SVC_SHA1Init( SVCSHA1Context *ctx );
void SVC_SHA1Update( SVCSHA1Context *ctx, const unsigned char *data, unsigned long len );
void SVC_SHA1GetHash( SVCSHA1Context *ctx, unsigned char *md );

void SVC_CalcSHA1(
                    void*         buffer,     //  �o�͗̈�
                    const void*   buf,        //  �f�[�^�ւ̃|�C���^
                    unsigned int  len         //  �f�[�^�̒���
                    );

int SVC_CompareSHA1(
                    const void* decedHash,    //  SVC_Decrypto*�̏o��
                    const void* digest        //  SVC_GetDigest�̏o��
                    );

int SVC_RandomSHA1(
                    void*           dest_ptr,   // �o�̓f�[�^�ւ̃|�C���^
                    unsigned int    dest_len,   // �o�̓f�[�^�̒���
                    const void*     src_ptr,    // ���̓f�[�^�ւ̃|�C���^
                    unsigned int    src_len     // ���̓f�[�^�̒���
                    );

s32 SVC_UncompressLZ8FromDevice( const void* srcp,
                                  void* destp,
                                  const void* paramp,
                                  const MIReadStreamCallbacks *callbacks
                                  );

s32 SVC_UncompressLZ16FromDeviceIMG( const void* srcp,
                                  void* destp,
                                  const void* paramp,
                                  const MIReadStreamCallbacks *callbacks
                                  );

void SVC_HMACSHA1Init( SVCHMACSHA1Context *ctx, const void *key, u32 keylen );
void SVC_HMACSHA1Update( SVCHMACSHA1Context *ctx, const void *data, u32 len );
void SVC_HMACSHA1GetHash( SVCHMACSHA1Context *ctx, u8* md );
void SVC_CalcHMACSHA1( void* md, const void* data, u32 len, void* key, u32 keylen );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEMCALL_H_ */
#endif
