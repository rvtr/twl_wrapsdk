#!/usr/bin/perl --

use strict;

# 固定フォーマット
my $file_head_format =<<'EOF';
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libralies - camera
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

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#endif

static BOOL CAMERAi_S_Initialize( CameraSelect camera )
{
EOF

my $file_foot_format =<<'EOF';
    return TRUE;
}
EOF

my $single_format =<<'EOF';
    if (CAMERAi_S_WriteRegister(camera, 0x%02X, 0x%02X) == FALSE)
    {
        DBG_PRINTF("Failed to write a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $multi_head_format =<<'EOF';
    {
        const u8 data[] =
        {
EOF

my $multi_foot_format =<<'EOF';
        };
        if (CAMERAi_S_WriteRegisters(camera, 0x%02X, data, %d) == FALSE)
        {
            DBG_PRINTF("Failed to write registers! (%%d)\n", __LINE__);
            return FALSE;
        }
    }
EOF

my $row_nums = 8;
sub sprint_command {
	my($addr, @value) = @_;
	if (@value == 1) {					# シングルライトは別枠
		return sprintf($single_format, $addr, $value[0]);
	}
	my $result = $multi_head_format;	# これ以降はバーストライト
	for (my $i = 0; $i < @value; $i++)
	{
		if (($i % $row_nums) == 0) {
			$result .= "            " ;
		} else {
			$result .= " " ;
		}
		$result .= sprintf("0x%02X,", $value[$i]);
		if (($i % $row_nums) == ($row_nums - 1) ) {
			$result .= "\r\n";
		}
	}
	if ((@value % $row_nums) != 0) {
		$result .= "\r\n";
	}
	$result .= sprintf($multi_foot_format, $addr, scalar(@value));
	return $result;
}

#
# データはいったんキャッシュして、連続アドレスをバーストライトに書き換える
#
my $comment;					# コメントキャッシュ
my @cache;						# キャッシュデータ
my $start = -1;					# キャッシュの先頭アドレス
my @output;						# 出力データ

#ここからメイン

# 引数チェック
die "USAGE: $0 INFILE [OUTFILE]\n" if ($#ARGV != 1 and $#ARGV != 0);

# 各種初期化
my $infile = $ARGV[0];
my $outfile = $ARGV[1];
($outfile = $infile) =~ s/\.dat$/.autogen.c/ unless ($outfile);

# 入出力ファイルのオープン (両方オープンしておく)
open IN, $infile or die "Cannot open the input file!\n";
open OUT, ">", $outfile or die "Cannot open the output file!\n";

# 入力処理
while (<IN>) {
	s/[\r\n]+$//;	# delete \r and/or \n
	s|\#|// |g;		# change comment sign
	if (s|(//.*)||) {						# コメント抽出
		$comment .= "    $1\r\n";			# 独立行として出力予定
	}

	if (/\s*([\w\d]{2})\s+([\w\d]{2})/) {	# データ抽出
		my ($addr, $value) = (hex($1), hex($2));
		if ($addr != $start + @cache)		# アドレスが連続していないなら
		{									# 直前までを出力
			push @output, sprint_command($start, @cache) if (@cache);
			@cache = ($value);				# 最新の値だけのエントリにする
			$start = $addr;
		} else {							# アドレスが連続しているなら
			push @cache, $value;			# 値を追記
		}
		push @output, $comment;				# このタイミングでコメント出力
		$comment = "";
	}
	elsif (/\S/) {							# 未知入力行検出 (エラーにすべきかも)
		warn "UNKNOWN STATEMENT: <<", $_, ">>\n";
	}
}
# 最終行処理
push @output, sprint_command($start, @cache) if (@cache);
# 入力処理終了
close IN;

# 出力処理
$outfile =~ s/^.*[\\\/]//;	# get basename to print
printf OUT $file_head_format, $outfile;		# ファイルヘッダ出力
print OUT @output;
printf OUT $file_foot_format;				# ファイルフッタ出力
# 出力処理終了
close OUT;
