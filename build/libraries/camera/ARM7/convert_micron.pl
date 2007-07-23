#!/usr/bin/perl --

use strict;

# ��`�t�@�C��
my $register_sdat = "MT9V113-REV1.sdat";

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

static inline BOOL CAMERAi_M_WriteMCU( CameraSelect camera, u16 addr, u16 value )
{
    return CAMERAi_M_WriteRegister(camera, 0x98C, addr)
        && CAMERAi_M_WriteRegister(camera, 0x990, value);
}
static inline BOOL CAMERAi_M_ReadMCU( CameraSelect camera, u16 addr, u16 *pValue )
{
    return CAMERAi_M_WriteRegister(camera, 0x98C, addr)
        && CAMERAi_M_ReadRegisters(camera, 0x990, pValue, 1);
}

EOF

my $file_foot_format =<<'EOF';
EOF

my $declare_format =<<'EOF';
static BOOL CAMERAi_M_%s( CameraSelect camera );
EOF

my $func_head_format =<<'EOF';
static BOOL CAMERAi_M_%s( CameraSelect camera )
{
EOF

my $func_foot_format =<<'EOF';
    return TRUE;
}
EOF

my $call_format =<<'EOF';
    if (CAMERAi_M_%1$s(camera) == FALSE)%2$s
    {
        DBG_PRINTF("Failed to call CAMERAi_M_%1$s! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $reg_format =<<'EOF';
    if (CAMERAi_M_WriteRegister(camera, %s, %s) == FALSE)%s
    {
        DBG_PRINTF("Failed to write a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $set_format =<<'EOF';
    if (CAMERAi_M_SetFlags(camera, %s, %s) == FALSE)%s
    {
        DBG_PRINTF("Failed to set a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $clear_format =<<'EOF';
    if (CAMERAi_M_ClearFlags(camera, %s, %s) == FALSE)%s
    {
        DBG_PRINTF("Failed to clear a register! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $delay_format =<<'EOF';
    OS_Sleep(%s);%s
EOF

my $pollreg_format =<<'EOF';
    timeout = %5$s;%6$s
    while (1)
    {
        u16 data;
        if (CAMERAi_M_ReadRegisters(camera, %1$s, &data, 1) == FALSE)
        {
            DBG_PRINTF("Failed to read a register! (%%d)\n", __LINE__);
            return FALSE;
        }
        if ((data & %2$s) %3$s)
        {
            if (--timeout)
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

my $mcu_format =<<'EOF';
    if (CAMERAi_M_WriteMCU(camera, %s, %s) == FALSE)%s
    {
        DBG_PRINTF("Failed to write a MCU! (%%d)\n", __LINE__);
        return FALSE;
    }
EOF

my $fieldset_format =<<'EOF';
    {%3$s
        u16 data;
        if (CAMERAi_M_ReadMCU(camera, %1$s, &data) == FALSE)
        {
            DBG_PRINTF("Failed to read a MCU! (%%d)\n", __LINE__);
            return FALSE;
        }
        if (CAMERAi_M_WriteMCU(camera, %1$s, (u16)(data | %2$s)) == FALSE)
        {
            DBG_PRINTF("Failed to write a MCU! (%%d)\n", __LINE__);
            return FALSE;
        }
    }
EOF

my $fieldclear_format =<<'EOF';
    {%3$s
        u16 data;
        if (CAMERAi_M_ReadMCU(camera, %1$s, &data) == FALSE)
        {
            DBG_PRINTF("Failed to read a MCU! (%%d)\n", __LINE__);
            return FALSE;
        }
        if (CAMERAi_M_WriteMCU(camera, %1$s, (u16)(data & ~%2$s)) == FALSE)
        {
            DBG_PRINTF("Failed to write a MCU! (%%d)\n", __LINE__);
            return FALSE;
        }
    }
EOF

my $pollfield_format =<<'EOF';
    timeout = %4$s;%5$s
    while (1)
    {
        u16 data;
        if (CAMERAi_M_ReadMCU(camera, %1$s, &data) == FALSE)
        {
            DBG_PRINTF("Failed to read a MCU! (%%d)\n", __LINE__);
            return FALSE;
        }
        if (data %2$s)
        {
            if (--timeout)
            {
                OS_Sleep(%3$s);
                continue;
            }
            DBG_PRINTF("Failed to poll a register! (%%d)\n", __LINE__);
            return FALSE;
        }
        break;
    }
EOF

my $comp_target =<<'EOF';
    if \(CAMERAi_M_WriteRegister\(camera\, 0[xX]0?98[cC]\, ([\w\d]+)\) == FALSE\)([^\r\n]+)
    \{
        DBG_PRINTF\(\"Failed to write a register\! \(\%d\)\\n\"\, __LINE__\)\;
        return FALSE\;
    \}
    if \(CAMERAi_M_WriteRegister\(camera\, 0[xX]0?990\, ([\w\d]+)\) == FALSE\)(?:\s*//([^\r\n]+))?
    \{
        DBG_PRINTF\(\"Failed to write a register\! \(\%d\)\\n\"\, __LINE__\)\;
        return FALSE\;
    \}
EOF

# API�ʃf�[�^�x�[�X
#    name: API�� (�ŏ���API����O�͖���)
#    data: API�{�� (������̌����`��)
#    comment: �R�����g�s�̃e���|���� (API�̍Ō�Ɉʒu����R�����g��API�O�ɏo�����߂̂���)
#    declare: API�{�̂̐擪�ɑ}������f�[�^ (1�̂�)
#
my @functions = ({name => "", data => "", comment => "", declare => ""});

# ��`�t�@�C���f�[�^�x�[�X
my %regmap;	# map register/regmap name to the address/mask

# ��`�t�@�C���f�[�^�x�[�X�̐���
sub regmap_init {
	my %mcu;
	open IN, $_[0] or die;
	# search space
	while(<IN>) {
		if (/^\[ADDR_SPACE\]/) {
			while (<IN>) {
				last if (/^\[END\]/);
				if (/^([\w\d]+)\s*=\s*\{MCU, (\d+),/) {	# only MCU is supported
					$mcu{$1} = $2;
				}
			}
		}
		if  (/^\[REGISTERS\]/) {
			my $lastreg = "";
			while (<IN>) {
				last if (/\[END\]/);
				if (/(^[\w\d]+)\s*=\s*\{(\w+?),\s*([\w\d]+?),/) {	# address entry
					if ($mcu{$3}) {	# MCU
						$regmap{$1}{0} = sprintf("0xA%1X%02X", $mcu{$3}, hex($2));
					}
					$lastreg = $1;
				}
				elsif (/^\s*\{([\w\d]+?),\s*(\w+?),/) {	# bitfield entry
					$regmap{$lastreg}{$1} = $2;
				}
			}
		}
	}
	close IN;
#	use Data::Dumper;
#	print Dumper(\%mcu, \%regmap);
}

# API���̐��`
sub name_conv {
	$_ = $_[0];
	s/\<o\>/Option/g;
	s/\%/Percent/g;
	s/\-/Minus/g;
	s/\+/Plus/g;
	s/\>/To/g;
	s/[\s\.\:\=\*\/\(\)]+/_/g;
	return $_;
}

# �e��R�}���h��C����ւ̕ϊ�
sub func_conv {
	my($key, $value, $comment) = @_;
	my @v = split /\s*\,\s*/, $value;			# split value
	$comment = "    " . $comment if ($comment);	# insert spaces
	if ($key eq "LOAD") {
		return sprintf($call_format, name_conv($value), $comment);
	}
	elsif ($key eq "REG") {			# address, value
		return sprintf($reg_format, $v[0], $v[1], $comment);
	}
	elsif ($key eq "BITFIELD") {	# address, mask, set/clear
		if ($v[2]) {
			return sprintf($set_format, $v[0], $v[1], $comment);
		} else {
			return sprintf($clear_format, $v[0], $v[1], $comment);
		}
	}
	elsif ($key eq "DELAY") {		# msec
		return sprintf($delay_format, $v[0], $comment);
	}
	elsif ($key eq "POLL_REG") {	# address, mask, condition, delay, timeout
		$v[3] =~ s/DELAY\s*=\s*//;
		$v[4] =~ s/TIMEOUT\s*=\s*//;
		${$functions[$#functions]}{declare} = "    int timeout;\r\n";
		return sprintf($pollreg_format, $v[0], @v[1..4], $comment);
	}
	elsif ($key eq "VAR8") {		# page, address, value
		return sprintf($mcu_format, sprintf("0xA%1X%02X", $v[0], hex($v[1])), $v[2], $comment);
	}
	elsif ($key eq "VAR") {			# page, address, value
		return sprintf($mcu_format, sprintf("0x2%1X%02X", $v[0], hex($v[1])), $v[2], $comment);
	}
	elsif ($key eq "FIELD_WR") {	# name, value OR name, mask, set/clear
		if ($v[2] eq "") {
			return sprintf($mcu_format, $regmap{$v[0]}{0}, $v[1], $comment);
		}
		if ($v[2]) {
			return sprintf($fieldset_format, $regmap{$v[0]}{0}, $regmap{$v[0]}{$v[1]}, $comment);
		} else {
			return sprintf($fieldclear_format, $regmap{$v[0]}{0}, $regmap{$v[0]}{$v[1]}, $comment);
		}
	}
	elsif ($key eq "POLL_FIELD") {	# name, condition, delay, timeout
		$v[2] =~ s/DELAY\s*=\s*//;
		$v[3] =~ s/TIMEOUT\s*=\s*//;
		${$functions[$#functions]}{declare} = "    int timeout;\r\n";
		return sprintf($pollfield_format, $regmap{$v[0]}{0}, @v[1..3], $comment);
	}
	return "// IGNORED: " . $key . "=" . $value . $comment . "\r\n";	# ���Ή��R�}���h
}

# �㏈�� (���API������)
sub comp_func {
	$_[0] =~ s/$comp_target/sprintf($mcu_format, $1, $3, $2 . $4)/eg;
	return $_[0];
}

#�������烁�C��

# �����`�F�b�N
die "USAGE: $0 INFILE [OUTFILE]\n" if ($#ARGV != 1 and $#ARGV != 0);

# �e�평����
regmap_init($register_sdat);

my $infile = $ARGV[0];
my $outfile = $ARGV[1];
($outfile = $infile) =~ s/\.ini$/.autogen.c/ unless ($outfile);

# ���o�̓t�@�C���̃I�[�v�� (�����I�[�v�����Ă���)
open IN, $infile or die "Cannot open the input file!\n";
open OUT, ">", $outfile or die "Cannot open the output file!\n";

# ���͏���
while (<IN>) {
	my $comment = "";
	s/[\r\n]+$//;	# delete \r and/or \n
	if (s/(\;|\/\/)(.*)//) {				# �R�����g���o
		$comment = $1 . $2;
	}
	if (/\s*\[(.+)\]/) {					# API���o
		push @functions, {name => name_conv($1), data => "", comment => "", declare => ""};
	}
	elsif (/\s*(.+?)\s*\=\s*(.+?)\s*$/) {	# �R�}���h���o
		# �R�}���h�̎�O�ɃR�����g���������ꍇ�́Adata�Ɉړ�������
		${$functions[$#functions]}{data} .= ${$functions[$#functions]}{comment};
		${$functions[$#functions]}{comment} = "";
		# �R�}���h��C����ɕϊ�����data�ɒǉ�
		${$functions[$#functions]}{data} .= func_conv($1, $2, $comment);
	}
	elsif (/\S+/) {							# ���m���͍s���o (�R�����g���� (�G���[�ɂ��ׂ�����))
		warn "UNKNOWN STATEMENT: <<", $_, ">>\n";
		${$functions[$#functions]}{comment} .= "// " . $_ . $comment . "\r\n";
	}
	elsif ($comment) {						# �R�����g�̂�
		${$functions[$#functions]}{comment} .= "// " . $comment . "\r\n";
	}
}
# ���͏����I��
close IN;

#use Data::Dumper;
#print Dumper(\@functions);

# �o�͏���
$outfile =~ s/^.*[\\\/]//;	# get basename to print
printf OUT $file_head_format, $outfile;		# �t�@�C���w�b�_�o��
foreach my $func ( @functions ) {			# ���o�ς�API�̐錾
	printf OUT $declare_format, $$func{name} if ($$func{name});
}
printf OUT "\r\n";
foreach my $func ( @functions ) {			# API�{�̂̏o��
	if ($$func{name}) {						# �ŏ���API���O�̏ꍇ������
		printf OUT $func_head_format, $$func{name};	# API�����J�����ʂ̏o��
		print OUT "#pragma unused(camera)\r\n" unless ($$func{data} =~ /camera/);	# warning�h�~
		print OUT $$func{declare}, "\r\n" if ($$func{declare});	# �ϐ��錾 (if any)
	}
	print OUT comp_func($$func{data});		# �{�̂̌㏈���{�o��
	if ($$func{name}) {						# �ŏ���API���O�̏ꍇ������
		printf OUT $func_foot_format ;		# �����ʏo��
	}
	print OUT $$func{comment};				# �Ō�̃R�����g�̏o�� (�����Ă�����API�̂��߂̂���)
}
printf OUT $file_foot_format;				# �t�@�C���t�b�^�o��
# �o�͏����I��
close OUT;

