#!/usr/bin/perl --

use strict;

# �Œ�t�H�[�}�b�g
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
	if (@value == 1) {					# �V���O�����C�g�͕ʘg
		return sprintf($single_format, $addr, $value[0]);
	}
	my $result = $multi_head_format;	# ����ȍ~�̓o�[�X�g���C�g
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
# �f�[�^�͂�������L���b�V�����āA�A���A�h���X���o�[�X�g���C�g�ɏ���������
#
my $comment;					# �R�����g�L���b�V��
my @cache;						# �L���b�V���f�[�^
my $start = -1;					# �L���b�V���̐擪�A�h���X
my @output;						# �o�̓f�[�^

#�������烁�C��

# �����`�F�b�N
die "USAGE: $0 INFILE [OUTFILE]\n" if ($#ARGV != 1 and $#ARGV != 0);

# �e�평����
my $infile = $ARGV[0];
my $outfile = $ARGV[1];
($outfile = $infile) =~ s/\.dat$/.autogen.c/ unless ($outfile);

# ���o�̓t�@�C���̃I�[�v�� (�����I�[�v�����Ă���)
open IN, $infile or die "Cannot open the input file!\n";
open OUT, ">", $outfile or die "Cannot open the output file!\n";

# ���͏���
while (<IN>) {
	s/[\r\n]+$//;	# delete \r and/or \n
	s|\#|// |g;		# change comment sign
	if (s|(//.*)||) {						# �R�����g���o
		$comment .= "    $1\r\n";			# �Ɨ��s�Ƃ��ďo�͗\��
	}

	if (/\s*([\w\d]{2})\s+([\w\d]{2})/) {	# �f�[�^���o
		my ($addr, $value) = (hex($1), hex($2));
		if ($addr != $start + @cache)		# �A�h���X���A�����Ă��Ȃ��Ȃ�
		{									# ���O�܂ł��o��
			push @output, sprint_command($start, @cache) if (@cache);
			@cache = ($value);				# �ŐV�̒l�����̃G���g���ɂ���
			$start = $addr;
		} else {							# �A�h���X���A�����Ă���Ȃ�
			push @cache, $value;			# �l��ǋL
		}
		push @output, $comment;				# ���̃^�C�~���O�ŃR�����g�o��
		$comment = "";
	}
	elsif (/\S/) {							# ���m���͍s���o (�G���[�ɂ��ׂ�����)
		warn "UNKNOWN STATEMENT: <<", $_, ">>\n";
	}
}
# �ŏI�s����
push @output, sprint_command($start, @cache) if (@cache);
# ���͏����I��
close IN;

# �o�͏���
$outfile =~ s/^.*[\\\/]//;	# get basename to print
printf OUT $file_head_format, $outfile;		# �t�@�C���w�b�_�o��
print OUT @output;
printf OUT $file_foot_format;				# �t�@�C���t�b�^�o��
# �o�͏����I��
close OUT;
