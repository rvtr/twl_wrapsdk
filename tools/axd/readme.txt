�E"export TWL_DEBUGGER=ARM" �܂��� "make TWL_DEBUGGER=ARM" �Ńr���h���ĉ������B

�E�u���b�h���[�h�ł� Multi-ICE �� Auto-Configure �������Ȃ��̂ŁA
  Load-Configuration �ɂ� multi-ice-twl.cfg �����[�h���ĉ������B

�E�f�X�N�g�b�v�� AXD-Debugger �̃V���[�g�J�b�g���쐬���A
  �u�����N��v�ɉ��L�̂悤�ɐݒ肷��ƕ֗��ł��B

ARM9���F

%ARMBIN_AXD% -nologo -session %TWLSDK_ROOT%/tools/axd/ARM9.ses -debug %TWLSDK_ROOT%/�i�N�����Ƀ��[�h����axf�t�@�C��)

ARM7���� -session �� "ARM7.ses" ���w�肵�܂��B
-debug �̑���� -exec ���g�p����ƋN������Ɏ��s����܂��B
