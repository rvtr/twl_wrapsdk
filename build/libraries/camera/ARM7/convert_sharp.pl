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

sub print_data {
	my $result = "";
	for (my $i = 0; $i < @_; $i++)
	{
		if (($i % 16) == 0) {
			$result .= "            " ;
		} else {
			$result .= " " ;
		}
		$result .= sprintf("0x%02X,", $_[$i]);
		if (($i % 16) == 15) {
			$result .= "\r\n";
		}
	}
	if ((@_ % 16) != 0) {
		$result .= "\r\n";
	}
	return $result;
}

sub print_command {
	my($addr, @value) = @_;
	if (@value == 1) {
		return sprintf($single_format, $addr, $value[0]);
	}
	my $result = $multi_head_format;
	for (my $i = 0; $i < @value; $i++)
	{
		if (($i % 16) == 0) {
			$result .= "            " ;
		} else {
			$result .= " " ;
		}
		$result .= sprintf("0x%02X,", $value[$i]);
		if (($i % 16) == 15) {
			$result .= "\r\n";
		}
	}
	if ((@value % 16) != 0) {
		$result .= "\r\n";
	}
	$result .= sprintf($multi_foot_format, $addr, scalar(@value));
	return $result;
}

#
# �f�[�^�͂�������L���b�V�����āA�A���A�h���X���o�[�X�g���C�g�ɏ���������
#
my @cache;						# �L���b�V���f�[�^
my $start = -1;					# �L���b�V���̐擪�A�h���X
my @output;						# �o�̓f�[�^
my $comment = "";				# �R�����g

#�������烁�C��

# �����`�F�b�N
die "USAGE: $0 INFILE [OUTFILE]\n" if ($#ARGV != 1 and $#ARGV != 0);

# �e�평����
my $infile = $ARGV[0];
my $outfile = $ARGV[1];
($outfile = $infile) =~ s/\.ini$/.autogen.c/ unless ($outfile);

# ���o�̓t�@�C���̃I�[�v�� (�����I�[�v�����Ă���)
open IN, $infile or die "Cannot open the input file!\n";
open OUT, ">", $outfile or die "Cannot open the output file!\n";

# ���͏���
while (<IN>) {
	s/[\r\n]+$//;	# delete \r and/or \n
	s|\#|// |g;		# change comment sign
	if (s|(//.*)||) {						# �R�����g���o
		$comment .= "    $1\r\n";
	}

	if (/\s*([\w\d]{2})\s+([\w\d]{2})/) {	# �f�[�^���o
		my ($addr, $value) = (hex($1), hex($2));
		if ($addr != $start + @cache)
		{
			push @output, print_command($start, @cache) if (@cache);
			@cache = ($value);
			$start = $addr;
		} else {
			push @cache, $value;
		}
		push @output, $comment;				# �R�����g�o��
		$comment = "";
	}
	elsif (/\S+/) {							# ���m���͍s���o (�R�����g���� (�G���[�ɂ��ׂ�����))
		warn "UNKNOWN STATEMENT: <<", $_, ">>\n";
	}
}
# �ŏI�s����
push @output, print_command($start, @cache);
# ���͏����I��
close IN;

# �o�͏���
$outfile =~ s/^.*[\\\/]//;	# get basename to print
printf OUT $file_head_format, $outfile;		# �t�@�C���w�b�_�o��
print OUT @output;
printf OUT $file_foot_format;				# �t�@�C���t�b�^�o��
# �o�͏����I��
close OUT;
