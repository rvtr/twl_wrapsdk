#! bash -f
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
#  ���[�U�̊J�����Ɉˑ������ݒ�
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#  CodeWarrior �̃C���X�g�[����
#----------------------------------------------------------------------------
export CW_2_0_twl='C:\Program Files\Freescale\CW for NINTENDO DS V2.0'

#----------------------------------------------------------------------------
#  CVS �T�[�o�̐ݒ�
#----------------------------------------------------------------------------
export SVNROOT=file:///\Aqua/svn/twl_wrapsdk
export SVNROOT_TWLSDK=$SVNROOT/trunk

#----------------------------------------------------------------------------
#  �r���h�f�B���N�g���ʒu
#----------------------------------------------------------------------------
export WORKSPACE_ROOT=$TWLSDK_ROOT/dev/autobuild

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
#  �ȉ����ʐݒ�l
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
export      BUILDER_ROOT="`cygpath -a $0 | sed 's/\(\/builder\)\/.*$/\1/'`"
export      PROJECT_ROOT="$BUILDER_ROOT/projects"
export               SVN="svn"

export   SENDREPORT_ROOT="$BUILDER_ROOT/sendreport"
export          SENDMAIL="$SENDREPORT_ROOT/sendreport.sh"
export        LOG_FILTER="$SENDREPORT_ROOT/sendreport-logfilter.awk"
export        MAKEHEADER="$SENDREPORT_ROOT/makeheader.sh"
export           MAKENOW="$SENDREPORT_ROOT/makenow.sh"
export SENDREPORT_TMPDIR="$SENDREPORT_ROOT/tmp"
export        ERRORS_LOG="$SENDREPORT_TMPDIR/errors.log"
export        TMP_REPORT="$SENDREPORT_TMPDIR/report.tmp"

#----------------------------------------------------------------------------
