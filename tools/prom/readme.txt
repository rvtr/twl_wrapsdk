exoファイル書き込み手順
1) Xilinx Platform Cable USB をブレッドボードのPROMコネクタへ接続。
2) ブレッドボードの電源を入れ、ケーブルの LED が緑へ変化したのを確認。
3) Xilinx iMPACT 起動。
4) create a new project->Finish でプロジェクトファイル作成。
5) PROMファイル選択、Select PROM->xc18v01_pc20 アサイン。
6) Output->Cable Setup->TCK Speed を 1.5MHz 以下に設定。
7) Program選択、PROM Specific Properties->Load FPGA チェッククリア、OK。
