#! bash
#########################################################################
#
#  Script to send report
#  ToAddress ファイル内のヘッダ記述を使い、ログファイルを smtp で送信する．
#
# $Id: sendreport.sh,v 1.7 2005/09/13 06:13:38 yasu Exp $
#
#  Usage:  % sendreport to_address logfile 
#
#########################################################################
source `dirname $0`/../etc/spec.sh

#
# 引数のチェック
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
# ファイル送信
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
# BuildModule: 行の削除
#
# Revision 1.6  2005/09/13 06:05:49  yasu
# Subject の追加
#
# Revision 1.5  2005/09/13 05:47:41  yasu
# 改行コードの取り扱いの修正 head コマンドは CR を削除する
#
# Revision 1.4  2005/09/13 04:19:22  yasu
# メール送信コマンドを ssmtp から blatj に変更
#
# Revision 1.3  2005/06/30 10:59:11  yasu
# 修正
#
# Revision 1.2  2005/06/30 10:49:17  yasu
# 修正
#
# Revision 1.1  2005/06/30 08:26:17  yasu
# ディレクトリ整理
#
# Revision 1.2  2005/06/29 12:57:59  yasu
# メール転送バグフィクス
#
# Revision 1.1  2005/06/28 09:26:49  yasu
# sendreport システムの作成
#
#########################################################################
