#! bash -f
#-----------------------------------------------------------------------------
#  ���[���̃w�b�_�o��
#	makeheader.sh BUILD_LEVEL PROJECT BRANCH
#-----------------------------------------------------------------------------
BUILD_LEVEL=$1
PROJECT=$2
BRANCH=$3
HOSTNAME=`hostname`
#
date +"Subject: ["$PROJECT"] "$BUILD_LEVEL"-"$BRANCH" %Y/%m/%d-%H:%M%n"
echo "***** This Mail was created by "$BUILD_LEVEL" on "$HOSTNAME". *****"
#
