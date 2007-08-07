#! gawk -f
#----------------------------------------------------------------------------
# File:     	loghacker.awk
# Description:	Hack the nightly build log file for sendmail
#----------------------------------------------------------------------------

# skip null line
/^$/		{ next; }

# cvs checkout log
/^U /		{ next; }

# cvs tag log
/^T /		{ next; }

# cvs ? log
/^[?] /		{ next; }

# Subject:
/^Subject: /	{ print $0"\n"; next; }

# succeeded install command
/^  install:/	{ next; }

# zipped 'adding file'
/^  adding:/				{ next; }
/^	zip warning: name not matched/	{ next; }

# warning build/demos/e3s/roundtrip/Makefile
/^Warning: This demo needs/		{ next; }

# success message by BinToElf(CodeWarrior)
/^Success!$/				{ next; }

# save current directory
/^====/		{ currentdir = $0"\n"; next; }

# title
/^(:::|\[\[\[)/	{ print "\n"$0; currentdir = ""; next; }

# error message
		{ printf currentdir; print $0; currentdir = ""; }

#----------------------------------------------------------------------------
# $Log: sendreport-logfilter.awk,v $
# Revision 1.1  2005/06/30 08:26:17  yasu
# ディレクトリ整理
#
# Revision 1.1  2005/06/28 09:26:49  yasu
# sendreport システムの作成
#
# $NoKeywords: $
#----------------------------------------------------------------------------
