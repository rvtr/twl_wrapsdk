=======================
AES�Ɋւ���|���V�[(��)
=======================

���݂̃��C�u���������͎��R�Ɉ������Ԃ����A�ŏI�I�ɂ�
�ȉ��̃|���V�[�𔽉f�����`�ɐ�����������B


��(�\�K)���̎��

����4�g�̌��̑g�ݍ��킹�����݂���B

    KEY[0]	ID[0]		SEED[0]
    KEY[1]	ID[1]		SEED[1]
    KEY[2]	ID[2]		SEED[2]
    KEY[3]	ID[3]		SEED[3]

��(�\�K)���̊�{����

���ꂼ�ꂪ�Ɨ��������W�X�^�ł��邪�ASEED��ݒ肵���Ƃ��ɁA
SEED + ID => KEY �Ƃ����v�Z�����������Ȃ��(�v�Z���͔閧)�B
�܂��A�C�ӂ̃^�C�~���O��KEY�̂ЂƂ�AES�R�A�ɑ��邱�Ƃ��ł���B
����AES�R�A�ɑ����Ƃ��s��Ȃ�����AES�Ŏg�p����錮��
�ύX����Ȃ��B

��(�\�K)��ʂ��Ƃ̈Ӗ�����

KEY�͈�ʓI��AES��H�Ƃ��đ��̃V�X�e���Ƃ���肷��悤��
�f�[�^����舵���Ƃ��ɂ̂ݎg�p����B

ID��SEED�͔C�V���ˑ��̂��Ƃ�Ŏg�����ƂɂȂ邪�A
��{�I��ID�̕��̓V�X�e�����ł��炩���ߐݒ肵�Ă����A
�A�v���P�[�V�����ɂ͑��삳���Ȃ��B


�����ꂼ��̌��̈Ӗ�����

        �f�o�C�X�ˑ�	�A�v���P�[�V�����ˑ�
    0:      �~                   ��
    1:      ��                   ��
    2:      �~                   �~
    3:      ��                   �~

�����ŁA�f�o�C�X�ˑ��Ƃ́AeFuse�̓��e�𔽉f����Ƃ������ƁB
�܂��A�A�v���P�[�V�����ˑ��Ƃ́AROM�w�b�_�ӂ�𔽉f����Ƃ������ƁB

��L�̓��e�𔽉f����悤�ɁAID��ݒ肷�邱�ƂɂȂ�B


���A�v���P�[�V�������G�邱�Ƃ̂ł��镔��

�A�v���P�[�V�����ɂ́AKEY[0]�ASEED[0]�`SEED[3]���g����悤�ɂ���B
�ꍇ�ɂ���ẮASEED[2]��SEED[3]�͉�������Ȃ�(2�����Ȃ�)


��ARM7��������

ROM�R�[�h�őS���W�X�^�̏����l��ݒ肵�Ă���B
�唼�̓_�~�[�����AID�֌W�̌Œ�l�͂����ł݂̂̐ݒ�ƂȂ�B

�A�v���P�[�V�������[�_�[�p�ɁATwlFirm����ID��game_code�ˑ�
�����݂̂��Đݒ肷��R�[�h��p�ӂ��Ă���B

�A�v���P�[�V�����N�����ID��G�邱�Ƃ͖����B


��ARM9��API

���̂悤�Ȍʂ�API��p�ӂ��Č����B

AES_SetGeneralKey()	KEY[0]�Ɍ���ݒ肷��
AES_SetSystemKey()	SEED[3]�Ɍ���ݒ肷��
AES_SetGameKey()	SEED[0]�Ɍ���ݒ肷��
AES_SetSpecialKey()	SEED[1]�Ɍ���ݒ肷��
//AESi_SetAlternativeKey()	SEED[2]�Ɍ���ݒ肷�� (����J)

���ꂼ������������ɗL���ɂ�����̂ŁA�u�ȑO�̌��v�Ƃ����g������
�ł��Ȃ��悤�ɂ��Ă���B


��ARM7��API

�ʂɎw�肷�邱�Ƃ���LAPI���g�p���邱�Ƃ��ł���B
�ʐݒ�p�ɁA����enum��p�ӂ��Ă���B
typedef enum
{
    AES_KEYSEL_GAME         = 0,
    AES_KEYSEL_SPECIAL      = 1,
    AES_KEYSEL_ALTERNATIVE  = 2,
    AES_KEYSEL_SYSTEM       = 3,

    AES_KEYSEL_IPL          = AES_KEYSEL_ALTERNATIVE,

    AES_KEYSEL_GENERAL      = 0 // for key register
}
AESKeySel;
