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

SEND_TO=`gawk 'NR>=2{printf",CC:";} NR>=1{printf $0;}' ${ADR_FILE}`
SUBJECT=`gawk 'NR==1{gsub(/^[A-Za-z]*: */, ""); printf $0;}' ${LOG_FILE}`
gawk -f ${LOG_FILTER} ${LOG_FILE} | sed -e '/^[^:]\+$/,1000!d' >${TMP_REPORT}

TMP_REPORT_WIN=`cygpath -m ${TMP_REPORT}`
${SENDREPORT_ROOT}/smail.exe -hayame.nintendo.co.jp -f"AUTOBUILDER <okubata_ryoma@nintendo.co.jp>" -s"${SUBJECT}" -F"${TMP_REPORT_WIN}" "${SEND_TO}"


# 4.[使用方法]
# 
#    4-1 コマンド：smail.exe
# 
# 　 smail [-d][-t][-i][-S][-p(1～9)] -hホスト名 -f送信ユーザ名 -sサブジェクト
#    -F本文用ファイル名 -a添付ファイル名1[,添付ファイル名2,..]  user1@XXXX[,user2@XXX,BCC:user3@XXX..]
# 
#    4-2 オプション一覧
# 
#     ---------------------------------------------------------------------------------
# 	 *  -h: メールサーバ名又はIPアドレス
# 　   	 *  user1@xxx.co.jp,[[BCC:]user2@xxx.co.jp],... 
#          送信先メールアドレス（カンマ区切りで複数指定可能）
#          CC:をメールアドレス頭に付加したものは、CC扱いとなります。
#          BCC:をメールアドレス頭に付加したものは、BCC扱いとなります。
# 　　---------------------------------------------------------------------------------
# 	    -f: 送信者名   :  -f"Eva <info@picolix.jp>"   ←ニックネームを付けたい時
#                            :  -finfo@picolix.jp
# 	    -s: サブジェクト
# 	    -F: 本文内容テキストファイル名
# 		-T: 本文内容 -Fよりも優先される
# 	    -a: 添付ファイル名 (カンマ区切りで複数可能)
# 		-p: プライオリティー -p4以下は、重要度高 -p5以上は重要度低　　: -p1 ～ -p9
# 	    -t: NTタイムゾーン調整オプション
#             -S: サイレントモード（エラー表示しない）
# 	    -d: デバッグ表示モード
# 	    -i: インフォメーション表示モード 
# 		-m: Message-IDの付加 -mで自動 -mxxxxxxxでxxxxxxxを付加
# 		-?: オプションヘルプ
# 
#     *部は必須です。
# 
#   4.3 実行例
# 
# 　(1) smail -hmailhost -fuser -sメールテスト -Fread.me foo@hoge.co.xx,foo1@hoge.co.xx -t
# 　(2) smail -hxxx.yyy.zzz.www -fuser -sメールテスト -Fread.me foo@hoge.co.jp,foo1@hoge.co.jp -t
# 　(3) smail -hxxx.yyy.zzz.www -fアカウント -Fread.me -sテスト -F本文.txt foo@hoge.co.jp
# 　(3) smail -hxxx.xxx.xxx.xxx -i -s"SMAILのテスト 　エクセル添付です。" 
# 　    -Fc:\temp\test.me -ac:\temp\test.xls -fxxx@xxx.xxx.xx yyy@yyy.yyy.xx,zzz@zzz.zzz.xx -t
#   (4) smail -hxxx.xxx.xxx.xxx -i -s"SMAILのテスト  エクセルとワードファイル添付です。" 
#       -Fc:\temp\test.me -ac:\temp\テスト.xls,c:\temp\サンプル.doc -fxxx@xxx.xxx.xx yyy@yyy.yyy.xx,zzz@zzz.zzz.xx -t
#   (5) smail -hxxx.xxx.xxx.xxx -s"TEST" CC:who1@xxx.xxx.xxx,who2@xxx.xxx.xxx,CC:who3@xxx.xxx.xxx,who4@xxxx.xxx.xxx,BCC:who5@xxx.xxx.xxx,who6@xxx.xxx.xxx -f"eva<who7@xxx.xxx.xxx>"
#         この場合は、
#         通常送信先：who2@xxx.xxx.xxx,who4@xxxx.xxx.xxx,who6@xxx.xxx.xxx
#         CC送信先  ：who1@xxx.xxx.xxx,who3@xxx.xxx.xxx
#         BCC送信先 ：who5@xxx.xxx.xxx
#         となります。
# 

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
