#! bash -f
#----------------------------------------------------------------------------
#  時短ビルド処理 quickbuild.sh
#	複数のブランチをビルドしたいのならここに追加する
#----------------------------------------------------------------------------
ROOT=`dirname $0`
source $ROOT/../etc/spec.sh

#----------------------------------------------------------------------------
# HEAD
#
$ROOT/TwlSDK.sh    quickbuild HEAD

#----------------------------------------------------------------------------
# revision 272
#
#$ROOT/TwlSDK.sh    quickbuild 272

#----------------------------------------------------------------------------
