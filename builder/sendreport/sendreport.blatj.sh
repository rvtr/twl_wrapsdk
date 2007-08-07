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

SEND_TO=`gawk  'NR==1{printf $0;}' ${ADR_FILE}`
SEND_CC=`gawk  'NR>=3{printf",";} NR>=2{printf $0;}' ${ADR_FILE}`
REPLY_TO=`gawk 'NR>=2{printf",";} NR>=1{printf $0;}' ${ADR_FILE}`

SUBJECT=`gawk  'NR==1{gsub(/^[A-Za-z]*: */, ""); printf $0;}' ${LOG_FILE}`
gawk -f ${LOG_FILTER} ${LOG_FILE} | sed -e '2,1000!d;0,/^$/d;' >${TMP_REPORT}

TMP_REPORT_WIN=`cygpath -m ${TMP_REPORT}`
${SENDREPORT_ROOT}/blatj.exe "${TMP_REPORT_WIN}" -noh2 -t "${SEND_TO}" -c "${SEND_CC}" -s "${SUBJECT}" -server mail -port 25 -f 'AUTOBUILDER <yasu@nintendo.co.jp>'

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
