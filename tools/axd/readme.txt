・"export TWL_DEBUGGER=ARM" または "make TWL_DEBUGGER=ARM" でビルドして下さい。

・ブレッドモードでは Multi-ICE の Auto-Configure が働かないので、
  Load-Configuration にて multi-ice-twl.cfg をロードして下さい。

・デスクトップへ AXD-Debugger のショートカットを作成し、
  「リンク先」に下記のように設定すると便利です。

ARM9側：

%ARMBIN_AXD% -nologo -session %TWLSDK_ROOT%/tools/axd/ARM9.ses -debug %TWLSDK_ROOT%/（起動時にロードするaxfファイル)

ARM7側は -session へ "ARM7.ses" を指定します。
-debug の代わりに -exec を使用すると起動直後に実行されます。
