<h1 align="center"> OpenSpeedy </h1>

<p align="center">
  <img style="margin:0 auto" width=100 height=100 src="https://github.com/user-attachments/assets/bdbe4a60-7692-4e9c-9df4-ad1711337c57">
  </img>  
</p>

<p align="center">
  OpenSpeedyはオープンソースの無料ゲーム速度変更ツールで、フレームレート制限を突破し、よりスムーズなゲーム体験を提供します。
</p>

<p align="center">
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/v/release/game1024/OpenSpeedy?color=brightgreen" alt="バージョン">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/downloads/game1024/OpenSpeedy/total" alt="ダウンロード数">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy">
    <img src="https://img.shields.io/badge/Platform-Windows-lightblue" alt="プラットフォーム">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/commits">
    <img src="https://img.shields.io/github/commit-activity/m/game1024/OpenSpeedy" alt="コミット活動">
  </a>

  <img src="https://img.shields.io/badge/language-C/C++-blue">
  <img src="https://img.shields.io/badge/License-GPLv3-green.svg">
  <a href="https://github.com/game1024/OpenSpeedy/stargazers">
    <img src="https://img.shields.io/github/stars/game1024/OpenSpeedy" alt="GitHub Stars">
  </a>
</p>

<p align="center">
  <a href="https://github.com/game1024/OpenSpeedy">
    简体中文
  </a>
  ·
  <a href="https://github.com/game1024/OpenSpeedy/blob/master/docs/README_ja.md">
    日本語
  </a>
  ·
  <a href="https://github.com/game1024/OpenSpeedy/blob/master/docs/README_en.md">
    English
  </a>
</p>

# 🚀 特徴
- 完全無料でオープンソース
- シンプルで使いやすいインターフェース
- カスタマイズ可能な速度調整
- 様々なゲームエンジンとの互換性
- システムリソース使用量が少ない
- x86およびx64プラットフォームのプロセスを加速可能
- カーネル侵入なし、Ring3レイヤーHookでシステムカーネルの整合性を損なわない

# 📥 インストール
1. [リリースページ](https://github.com/game1024/OpenSpeedy/releases)から最新バージョンをダウンロード
2. ダウンロードしたファイルを任意の場所に解凍
3. インストール不要、OpenSpeedy.exeを実行するだけで使用可能

# 💻 システム要件
- OS: Windows 10以上
- プラットフォーム: x86（32ビット）およびx64（64ビット）

# 📝 使用方法
1. OpenSpeedyを起動
2. 速度変更したい対象ゲームを実行
<img src="https://github.com/user-attachments/assets/648e721d-9c3a-4d82-954c-19b16355d084" width="50%">

3. ゲームプロセスを選択し、OpenSpeedyインターフェースで速度倍率を調整
<img src="https://github.com/user-attachments/assets/73ef1a95-64fb-4017-939c-a44405a6e742" width="50%">

4. 即時に反映されます。比較効果は以下の通り：

<video src="https://github.com/user-attachments/assets/74471b1f-7f95-4de8-b5aa-7edc85c9d5f0" width="70%"></video>

# 🔧 技術原理
OpenSpeedyは以下のWindowsシステム時間関数をフックすることでゲーム速度調整を実現しています：

|関数名	| ライブラリ |	機能 |
|--------|----------|------------------|
|Sleep|user32.dll|スレッドスリープ|
|SetTimer|user32.dll|メッセージベースのタイマーを作成|
|timeGetTime | winmm.dll	| システム起動後の経過ミリ秒数を取得 |
|GetTickCount | kernel32.dll	| システム起動後の経過ミリ秒数を取得 |
|GetTickCount64	| kernel32.dll	| システム起動後の経過ミリ秒数を取得（64ビット） |
|QueryPerformanceCounter |	kernel32.dll	| 高精度パフォーマンスカウンター |
|GetSystemTimeAsFileTime |	kernel32.dll	| システム時間を取得 |
|GetSystemTimePreciseAsFileTime |	kernel32.dll	| 高精度システム時間を取得 |

# ⚠️ 注意事項
- このツールは学習および研究目的のみに使用してください
- 一部のオンラインゲームにはアンチチートシステムがあり、このツールの使用によりアカウントが禁止される可能性があります
- 過度の加速はゲームの物理エンジンの異常やクラッシュを引き起こす可能性があります
- 競技性の高いオンラインゲームでの使用は推奨されません

# 🔄 フィードバック
OpenSpeedyの使用中に問題が発生した場合は、以下の方法でフィードバックをお寄せください：
- [GitHub Issues](https://github.com/game1024/OpenSpeedy/issues) - 問題報告の提出

# 📜 ライセンス
OpenSpeedyはGNU GPLv3ライセンスの下でリリースされています。

# 🙏 謝辞
OpenSpeedyは以下のプロジェクトのソースコードを使用しています。オープンソースコミュニティの貢献に感謝します：
- [minhook](https://github.com/TsudaKageyu/minhook): API Hookingに使用
- [Qt](https://www.qt.io/): GUIフレームワーク

免責事項: OpenSpeedyは教育および研究目的のみを意図しています。ユーザーはこのソフトウェアの使用に関連するすべてのリスクと責任を負います。作者は、このソフトウェアの使用によって生じるいかなる損失または法的責任についても責任を負いません。

<p align="center">
  <img src="https://api.star-history.com/svg?repos=game1024/openspeedy&type=Date" Alt="Star History Chart">
</p>
