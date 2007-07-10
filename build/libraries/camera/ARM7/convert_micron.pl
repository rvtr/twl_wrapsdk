#!/usr/bin/perl --

use strict;

my $file_head_format =<<'EOF';
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     %1$s

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro/os/common/thread.h>
#include <twl/camera/ARM7/i2c_micron.h>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif

EOF

my $file_foot_format =<<'EOF';
EOF

my $declare_format =<<'EOF';
BOOL CAMERAi_M_%s( CameraSelect camera );
EOF

my $func_head_format =<<'EOF';
BOOL CAMERAi_M_%s( CameraSelect camera )
{
EOF

my $func_foot_format =<<'EOF';
    return TRUE;
}
EOF

my $call_format =<<'EOF';
    if (CAMERAi_M_%1$s(camera) == FALSE) {%2$s
        DBG_PRINTF("Failed to call CAMERAi_M_%1$s! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $reg_format =<<'EOF';
    if (CAMERAi_M_WriteRegister(camera, %s, %s) == FALSE) {%s
        DBG_PRINTF("Failed to write a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $set_format =<<'EOF';
    if (CAMERAi_M_SetFlags(camera, %s, %s) == FALSE) {%s
        DBG_PRINTF("Failed to set a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $clear_format =<<'EOF';
    if (CAMERAi_M_ClearFlags(camera, %s, %s) == FALSE) {%s
        DBG_PRINTF("Failed to clear a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $sleep_format =<<'EOF';
    OS_Sleep(%s);%s
EOF

my $poolreg_format =<<'EOF';
    i = %5$s;%6$s
    while (1)
    {
    	u16 data;
        if (CAMERAi_M_ReadRegisters(camera, %1$s, &data, 1) == FALSE) {
            DBG_PRINTF("Failed to read a register! (%%d)\n", __LINE__);
            return FALSE;
        }
        if ((data & %2$s) %3$s)
        {
            if (--i)
            {
                OS_Sleep(%4$s);
                continue;
            }
            DBG_PRINTF("Failed to poll a register! (%%d)\n", __LINE__);
            return FALSE;
        }
        break;
    }
EOF

my @functions = ({name => "", data => "", declare => ""});	# API

sub name_conv {
	$_ = $_[0];
	s/\>/To/g;
	s/[\s\.\:\+\-\=\*\/]+/_/g;
	return $_;
}

sub func_conv {
	my($key, $value, $comment) = @_;
	$comment = "    " . $comment;	# insert spaces
	if ($key eq "LOAD") {
		return sprintf($call_format, name_conv($value), $comment);
	}
	elsif ($key eq "REG") {
		my($reg, $val) = split /\s*\,\s*/, $value;
		return sprintf($reg_format, $reg, $val, $comment);
	}
	elsif ($key eq "BITFIELD") {
		my($reg, $mask, $which) = split /\s*\,\s*/, $value;
		if ($which) {
			return sprintf($set_format, $reg, $mask, $comment);
		} else {
			return sprintf($clear_format, $reg, $mask, $comment);
		}
	}
	elsif ($key eq "DELAY") {
		return sprintf($sleep_format, $value, $comment);
	}
	elsif ($key eq "POLL_REG") {
		my($reg, $mask, $cond, $delay, $timeout) = split /\s*\,\s*/, $value;
		$delay =~ s/DELAY\s*=\s*//;
		$timeout =~ s/TIMEOUT\s*=\s*//;
		${$functions[$#functions]}{declare} = "    int i;\r\n";
		return sprintf($poolreg_format, $reg, $mask, $cond, $delay, $timeout, $comment);
	}
	return "    // " . $key . "=" . $value . $comment . "\r\n";
}

die "USAGE: convert.pl [INFILE] > [OUTFILE]\n" if ($#ARGV != 0);

my $infile = $ARGV[0];
(my $outfile = $infile) =~ s/\.ini$/.autogen.c/;

open IN, $infile or die "Cannot open the file!\n";

my @packets;        # パケットヘッダ＋パケットの中身の集まり
my @data;           # データ群

my $first = -1;  	# 先頭アドレス
my $current = -1;   # 期待アドレス

while (<IN>) {
	my $comment = "";
	s/[\r\n]+$//;	# delete \r and/or \n
	s|\;|//|;		# replace first ; to //
	if (s|(//.*)||) {
		$comment = $1;
	}
	if (/\s*\[(.+)\]/) {
		push @functions, {name => name_conv($1), data => "", declare => ""};
	}
	elsif (/\s*(.+?)\s*\=\s*(.+?)\s*$/) {
		${$functions[$#functions]}{data} .= func_conv($1, $2, $comment);
	}
	elsif (/\S+/) {
		print "UNKNOWN STATEMENT: <<", $_, ">>\n";
		${$functions[$#functions]}{data} .= $_ . $comment . "\r\n";
	}
	elsif ($comment) {
		${$functions[$#functions]}{data} .= $comment . "\r\n";
	}
}
close(IN);

#use Data::Dumper;
#print Dumper(\@functions);
#exit(1);

# output
printf $file_head_format, $outfile;
foreach my $func ( @functions ) {
	if ($$func{name}) {
		if ($$func{data} !~ /camera/) {
			$$func{declare} = "#pragma unused(camera)\r\n";
		}
		printf $declare_format, $$func{name};
	}
}
printf "\r\n";
foreach my $func ( @functions ) {
	if ($$func{name}) {
		printf $func_head_format, $$func{name};
		print $$func{declare}, "\r\n" if ($$func{declare});
	}
	print $$func{data};
	if ($$func{name}) {
		printf $func_foot_format;
	}
}
printf $file_foot_format;

