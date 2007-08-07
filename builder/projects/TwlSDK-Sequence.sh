#! bash
#----------------------------------------------------------------------------
#  TwlSDK.sh [nightlybuild/quickbuild] [HEAD/1_00pr1/...]
#----------------------------------------------------------------------------
source `dirname $0`/../etc/spec.sh

TARGET=$1
BRANCH=$2
PKNAME=twl_wrapsdk
MAJORVER=5

case ${BRANCH} in
HEAD)
	BRANCH_DIR=HEAD
	BRANCH_TAG=HEAD
	;;
*)
	BRANCH_DIR=${BRANCH}
	BRANCH_TAG=${BRANCH}
	;;
esac

LOG_FILE=${WORKSPACE_ROOT}/${PKNAME}/${TARGET}-${BRANCH}.log
ADR_FILE=${BUILDER_ROOT}/etc/address/${TARGET}.adr

#--- twl_wrapsdk/release ディレクトリをチェックアウト
mkdir -p ${WORKSPACE_ROOT}/${PKNAME}
cd       ${WORKSPACE_ROOT}/${PKNAME}
rm   -rf ${BRANCH_DIR}

export SVNROOT=${SVNROOT_TWLSDK}
echo ${SVN} export -r ${BRANCH_TAG} ${SVNROOT}/release ${BRANCH_TAG}/${PKNAME}/release
${SVN} --quiet export -r ${BRANCH_TAG} ${SVNROOT}/release ${BRANCH_TAG}/${PKNAME}/release

#--- ビルドを行ないログを記録する
( ${MAKEHEADER} ${TARGET} ${PKNAME} ${BRANCH} && \
  ${MAKENOW} && \
  make -sk -f ./${PKNAME}/release/Makefile_twl.${MAJORVER}x -C ${BRANCH_DIR} TWLSDK_RELEASE=${WORKSPACE_ROOT}/${PKNAME}/${BRANCH_DIR} ${TARGET} && \
  ${MAKENOW} ) >${LOG_FILE} 2>&1

#--- zip ファイルをコピーする
mv ${WORKSPACE_ROOT}/${PKNAME}/${BRANCH_DIR}/*.zip ${WORKSPACE_ROOT}

#--- ログ転送
if [ -e ${ADR_FILE} ]; then
	${SENDMAIL} ${ADR_FILE} ${LOG_FILE}
fi

#----------------------------------------------------------------------------
