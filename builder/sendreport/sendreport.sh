#! bash
#########################################################################
#
#  Script to send report
#  ToAddress �t�@�C�����̃w�b�_�L�q���g���A���O�t�@�C���� smtp �ő��M����D
#
# $Id: sendreport.sh,v 1.7 2005/09/13 06:13:38 yasu Exp $
#
#  Usage:  % sendreport to_address logfile 
#
#########################################################################
source `dirname $0`/../etc/spec.sh

#
# �����̃`�F�b�N
#
if [ "$1" == "" ]; then
  echo No ToAddress File
  exit 1
fi
export ADR_FILE=$1

if [ "$2" == "" ]; then
  echo No LogFile
  exit 1
fi
export LOG_FILE=$2

#
# �t�@�C�����M
#
mkdir -p ${SENDREPORT_TMPDIR}

SEND_TO=`gawk 'NR>=2{printf",CC:";} NR>=1{printf $0;}' ${ADR_FILE}`
SUBJECT=`gawk 'NR==1{gsub(/^[A-Za-z]*: */, ""); printf $0;}' ${LOG_FILE}`
gawk -f ${LOG_FILTER} ${LOG_FILE} | sed -e '/^[^:]\+$/,1000!d' >${TMP_REPORT}

TMP_REPORT_WIN=`cygpath -m ${TMP_REPORT}`
${SENDREPORT_ROOT}/smail.exe -hayame.nintendo.co.jp -f"AUTOBUILDER <okubata_ryoma@nintendo.co.jp>" -s"${SUBJECT}" -F"${TMP_REPORT_WIN}" "${SEND_TO}"


# 4.[�g�p���@]
# 
#    4-1 �R�}���h�Fsmail.exe
# 
# �@ smail [-d][-t][-i][-S][-p(1�`9)] -h�z�X�g�� -f���M���[�U�� -s�T�u�W�F�N�g
#    -F�{���p�t�@�C���� -a�Y�t�t�@�C����1[,�Y�t�t�@�C����2,..]  user1@XXXX[,user2@XXX,BCC:user3@XXX..]
# 
#    4-2 �I�v�V�����ꗗ
# 
#     ---------------------------------------------------------------------------------
# 	 *  -h: ���[���T�[�o������IP�A�h���X
# �@   	 *  user1@xxx.co.jp,[[BCC:]user2@xxx.co.jp],... 
#          ���M�惁�[���A�h���X�i�J���}��؂�ŕ����w��\�j
#          CC:�����[���A�h���X���ɕt���������̂́ACC�����ƂȂ�܂��B
#          BCC:�����[���A�h���X���ɕt���������̂́ABCC�����ƂȂ�܂��B
# �@�@---------------------------------------------------------------------------------
# 	    -f: ���M�Җ�   :  -f"Eva <info@picolix.jp>"   ���j�b�N�l�[����t��������
#                            :  -finfo@picolix.jp
# 	    -s: �T�u�W�F�N�g
# 	    -F: �{�����e�e�L�X�g�t�@�C����
# 		-T: �{�����e -F�����D�悳���
# 	    -a: �Y�t�t�@�C���� (�J���}��؂�ŕ����\)
# 		-p: �v���C�I���e�B�[ -p4�ȉ��́A�d�v�x�� -p5�ȏ�͏d�v�x��@�@: -p1 �` -p9
# 	    -t: NT�^�C���]�[�������I�v�V����
#             -S: �T�C�����g���[�h�i�G���[�\�����Ȃ��j
# 	    -d: �f�o�b�O�\�����[�h
# 	    -i: �C���t�H���[�V�����\�����[�h 
# 		-m: Message-ID�̕t�� -m�Ŏ��� -mxxxxxxx��xxxxxxx��t��
# 		-?: �I�v�V�����w���v
# 
#     *���͕K�{�ł��B
# 
#   4.3 ���s��
# 
# �@(1) smail -hmailhost -fuser -s���[���e�X�g -Fread.me foo@hoge.co.xx,foo1@hoge.co.xx -t
# �@(2) smail -hxxx.yyy.zzz.www -fuser -s���[���e�X�g -Fread.me foo@hoge.co.jp,foo1@hoge.co.jp -t
# �@(3) smail -hxxx.yyy.zzz.www -f�A�J�E���g -Fread.me -s�e�X�g -F�{��.txt foo@hoge.co.jp
# �@(3) smail -hxxx.xxx.xxx.xxx -i -s"SMAIL�̃e�X�g �@�G�N�Z���Y�t�ł��B" 
# �@    -Fc:\temp\test.me -ac:\temp\test.xls -fxxx@xxx.xxx.xx yyy@yyy.yyy.xx,zzz@zzz.zzz.xx -t
#   (4) smail -hxxx.xxx.xxx.xxx -i -s"SMAIL�̃e�X�g  �G�N�Z���ƃ��[�h�t�@�C���Y�t�ł��B" 
#       -Fc:\temp\test.me -ac:\temp\�e�X�g.xls,c:\temp\�T���v��.doc -fxxx@xxx.xxx.xx yyy@yyy.yyy.xx,zzz@zzz.zzz.xx -t
#   (5) smail -hxxx.xxx.xxx.xxx -s"TEST" CC:who1@xxx.xxx.xxx,who2@xxx.xxx.xxx,CC:who3@xxx.xxx.xxx,who4@xxxx.xxx.xxx,BCC:who5@xxx.xxx.xxx,who6@xxx.xxx.xxx -f"eva<who7@xxx.xxx.xxx>"
#         ���̏ꍇ�́A
#         �ʏ푗�M��Fwho2@xxx.xxx.xxx,who4@xxxx.xxx.xxx,who6@xxx.xxx.xxx
#         CC���M��  �Fwho1@xxx.xxx.xxx,who3@xxx.xxx.xxx
#         BCC���M�� �Fwho5@xxx.xxx.xxx
#         �ƂȂ�܂��B
# 

#########################################################################
# $Log: sendreport.sh,v $
# Revision 1.7  2005/09/13 06:13:38  yasu
# BuildModule: �s�̍폜
#
# Revision 1.6  2005/09/13 06:05:49  yasu
# Subject �̒ǉ�
#
# Revision 1.5  2005/09/13 05:47:41  yasu
# ���s�R�[�h�̎�舵���̏C�� head �R�}���h�� CR ���폜����
#
# Revision 1.4  2005/09/13 04:19:22  yasu
# ���[�����M�R�}���h�� ssmtp ���� blatj �ɕύX
#
# Revision 1.3  2005/06/30 10:59:11  yasu
# �C��
#
# Revision 1.2  2005/06/30 10:49:17  yasu
# �C��
#
# Revision 1.1  2005/06/30 08:26:17  yasu
# �f�B���N�g������
#
# Revision 1.2  2005/06/29 12:57:59  yasu
# ���[���]���o�O�t�B�N�X
#
# Revision 1.1  2005/06/28 09:26:49  yasu
# sendreport �V�X�e���̍쐬
#
#########################################################################
