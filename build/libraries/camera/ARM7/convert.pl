#!/usr/bin/perl --

my $support_multiple_write = undef;

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
#include <twl/camera/ARM7/i2c.h>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <twl/vlink.h>
#define DBG_PRINTF vlink_dos_printf
#define DBG_CHAR vlink_dos_put_console
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif

BOOL %2$s( void );
BOOL %2$s( void )
{
EOF

my $file_foot_format =<<'EOF';
    return TRUE;
}
EOF

my $packet_first_format =<<'EOF';
    {
        const u8 data[] = {
EOF

my $packet_last_format =<<'EOF';
        };
        if (CAMERAi_WriteRegisters(0x%s, data, %d) == FALSE) {
            DBG_PRINTF("Failed to initialize! (%%d)\n", __LINE__);
            return FALSE;
        }
    }
EOF

my $packet_single_format =<<'EOF';
    if (CAMERAi_WriteRegister(0x%s, 0x%s) == FALSE) {
        DBG_PRINTF("Failed to initialize! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

sub print_data {
    my $text = '';
    my $i = 0;
    foreach my $data (@_) {
        if ($i % 8 == 0) {
            $text .= '            ';
        } else {
            $text .= ' ';
        }
        $text .= sprintf('0x%s,', $data);
        $i++;
        if ($i % 8 == 0) {
            $text .= "\n";
        }
    }
    if ($i % 8 != 0) {
        $text .= "\n";
    }
    return $text;
}

sub print_single {
    return sprintf($packet_single_format, $_[0], $_[1]);
}

sub print_packet {
	my($first, @data) = @_;
	if (@data == 1) {
		return print_single($first, $data[0]);
	} else {
		return $packet_first_format,
			   print_data(@data),
			   sprintf($packet_last_format, $first, scalar(@data));
	}
}

die "USAGE: convert.pl [INFILE] > [OUTFILE]\n" if ($#ARGV != 0);

my $infile = $ARGV[0];
(my $funcname = $infile) =~ s/(_\d{6})?\.set$//;
$funcname =~ s/[^0-9a-zA-Z_]/_/g;	#
my $outfile = $funcname . ".autogen.c";
my $funcname = "CAMERAi_I2CPreset_" . $funcname;

open IN, $infile or die "Cannot open the file!\n";

my @packets;        # パケットヘッダ＋パケットの中身の集まり
my @data;           # データ群

my $first = -1;  	# 先頭アドレス
my $current = -1;   # 期待アドレス

if ($support_multiple_write) {
	while (<IN>) {
	    if (/^s([0-9a-f]{2})([0-9a-f]{2})/i) {
	        if ($current != hex($1)) {
	            if ($current >= 0) {
					push @packets, print_packet($first, @data);
	            }
	            $first = $1;
	            $current = hex($first);
	            @data = ();
	        }
	        push @data, $2;
	        $current++;
	    }
	}
	# last data
	push @packets, print_packet($first, @data);
} else {	# 連続書き込み非対応版
	while (<IN>) {
	    if (/^s([0-9a-f]{2})([0-9a-f]{2})/i) {
			push @packets, print_packet($1, $2);
		}
	}
}
close(IN);

# output
printf $file_head_format, $outfile, $funcname;
print @packets;
printf $file_foot_format;
