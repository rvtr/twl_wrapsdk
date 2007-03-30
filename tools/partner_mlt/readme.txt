Partner のプロジェクトファイルに core0、core1 を上書きして下さい。

現状では ARM7 側のロード直後にレジスタウインドウの CPSR をクリックして
SVCモードへ切り替えて下さい。

デフォルトは Partner 用にビルドされる設定になっていますが、変更された場合は
"export TWL_DEBUGGER=KMC" または "make TWL_DEBUGGER=KMC" でビルドして下さい。
