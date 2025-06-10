<h1 align="center"> OpenSpeedy </h1>

<p align="center">
  <img style="margin:0 auto" width=100 height=100 src="https://github.com/user-attachments/assets/bdbe4a60-7692-4e9c-9df4-ad1711337c57">
  </img>  
</p>

<p align="center">
  OpenSpeedyは、フレームレート制限を突破し、よりスムーズでシルキーなゲーム加速体験を提供するオープンソース無料のゲーム速度変更ツールです。
</p>

<p align="center">
  <img src="https://api.visitorbadge.io/api/visitors?path=game1024.openspeedy&countColor=%234ecdc4">
  
  <a href="https://github.com/game1024/OpenSpeedy/stargazers">
    <img src="https://img.shields.io/github/stars/game1024/OpenSpeedy?style=for-the-badge&color=yellow" alt="GitHub Stars">
  </a>

  <img src="https://img.shields.io/github/forks/game1024/OpenSpeedy?style=for-the-badge&color=8a2be2" alt="GitHub Forks">
  <br/>  
  
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/downloads/game1024/OpenSpeedy/total?style=for-the-badge" alt="Downloads">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/v/release/game1024/OpenSpeedy?style=for-the-badge&color=brightgreen" alt="Version">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy">
    <img src="https://img.shields.io/badge/Platform-Windows-lightblue?style=for-the-badge" alt="Platform">
  </a>
  <br/>
  
  <a href="https://github.com/game1024/OpenSpeedy/commits">
    <img src="https://img.shields.io/github/commit-activity/m/game1024/OpenSpeedy?style=for-the-badge" alt="提交活跃度">
  </a>
  <img src="https://img.shields.io/badge/language-C/C++-blue?style=for-the-badge">
  <img src="https://img.shields.io/badge/License-GPLv3-green.svg?style=for-the-badge">
  <br/>
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
- カスタマイズ可能な速度倍率
- 様々なゲームエンジンとの高い互換性
- 低いシステムリソース使用量
- x86とx64プラットフォームプロセスの同時加速対応
- カーネル非侵入性、Ring3レベルHook、システムカーネルを破壊しない

# 📥 インストール
1. [リリースページ](https://github.com/game1024/OpenSpeedy/releases)にアクセスして最新版をダウンロード
2. ダウンロードしたファイルパッケージを任意の場所に解凍
3. インストール不要、OpenSpeedy.exeを直接実行して使用可能

# 💻 システム要件
- OS: Windows 10以上
- プラットフォーム：x86（32ビット）およびx64（64ビット）

# 📝 使用方法
1. OpenSpeedyを起動
2. 速度変更したい対象ゲームを実行
<img src="https://github.com/user-attachments/assets/648e721d-9c3a-4d82-954c-19b16355d084" width="50%">

3. ゲームプロセスにチェックを入れ、OpenSpeedyインターフェースで速度倍率を調整
<img src="https://github.com/user-attachments/assets/9469aae9-8be0-4e40-884d-1fbea3206e73" width="50%">

4. 即座に効果が適用、比較効果は以下の通り

<video src="https://github.com/user-attachments/assets/74471b1f-7f95-4de8-b5aa-7edc85c9d5f0" width="70%"></video>

# 🔧 技術原理
OpenSpeedyは以下のWindowsシステム時間関数をHookすることでゲーム速度調整を実現：

| 関数名 | 所属ライブラリ | 機能 |
|--------|-------------|------|
| Sleep | user32.dll | スレッドスリープ |
| SetTimer | user32.dll | メッセージベースタイマーの作成 |
| timeGetTime | winmm.dll | システム起動後の経過ミリ秒数取得 |
| GetTickCount | kernel32.dll | システム起動後の経過ミリ秒数取得 |
| GetTickCount64 | kernel32.dll | システム起動後の経過ミリ秒数取得（64ビット） |
| QueryPerformanceCounter | kernel32.dll | 高精度パフォーマンスカウンター |
| GetSystemTimeAsFileTime | kernel32.dll | システム時間取得 |
| GetSystemTimePreciseAsFileTime | kernel32.dll | 高精度システム時間取得 |

# ⚠️ 注意事項
- 本ツールは学習・研究目的でのみ使用してください
- 一部のオンラインゲームにはアンチチートシステムがあり、本ツールの使用によりアカウントBANの可能性があります
- 過度の加速はゲーム物理エンジンの異常やクラッシュを引き起こす可能性があります
- 競技性のあるオンラインゲームでの使用は推奨しません

# 🔄 フィードバック
使用中に問題が発生した場合は、以下の方法でフィードバックをお願いします：
- [GitHub Issues](https://github.com/game1024/OpenSpeedy/issues) - 問題報告の提出

# 📜 ライセンス
OpenSpeedyはGNU v3ライセンスに従います。

# 🙏 謝辞
OpenSpeedyは以下のプロジェクトのソースコードを使用しており、オープンソースコミュニティの力に感謝します：
- [minhook](https://github.com/TsudaKageyu/minhook): API Hookに使用
- [Qt](https://www.qt.io/): GUIフレームワーク

免責事項: OpenSpeedyは教育・研究目的のみを意図しています。ユーザーは本ソフトウェアの使用に関するすべてのリスクと責任を負うものとします。作者は本ソフトウェアの使用により生じるいかなる損失や法的責任についても責任を負いません。

<p align="center">
  <img src="https://api.star-history.com/svg?repos=game1024/openspeedy&type=Date" Alt="Star History Chart">
</p>
