#! bash
#----------------------------------------------------------------------------
#  Nightly �r���h����
#	nightlybuild.sh [rev�ԍ�] ...
#	rev �ԍ��͕����w��ł���
#	���ϐ���K���ɐݒ肵�Ă��� TwlSDK-Sequence.sh ���Ăяo��
#----------------------------------------------------------------------------
ROOT=`dirname $0`
source ${ROOT}/../etc/spec.sh
TARGET=$1
PKNAME=TwlSDK
shift 1

tmp=`pwd`

for rev in $*; do
	case $rev in
	HEAD)
		# Build for HEAD
		echo === Build ${PKNAME} revision [$rev]
		export CWFOLDER_TWL=${CW_2_0_twl}
		export CWFOLDER_TWL_LONGJUMP=${CW_2_0_twl}
		export TWL_STD_PCHDR=True
		${ROOT}/${PKNAME}-Sequence.sh ${TARGET} $rev
		;;
	272)
		# Build for r272
		echo === Build ${PKNAME} revision [$rev]
		export CWFOLDER_TWL=${CW_2_0_twl}
		export CWFOLDER_TWL_LONGJUMP=${CW_2_0_twl}
		export TWL_STD_PCHDR=True
		${ROOT}/${PKNAME}-Sequence.sh ${TARGET} $rev
		;;
	*)
		# Unknown
		echo skip revision [$rev]
		;;
	esac
done

#----------------------------------------------------------------------------
