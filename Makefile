#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - build
# File:     Makefile
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: $
# $NoKeywords: $
#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

SUBDIRS =	\
            build \

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
