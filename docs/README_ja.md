<h1 align="center"> OpenSpeedy </h1>

<p align="center">
  <img style="margin:0 auto" width=100 height=100 src="https://github.com/user-attachments/assets/bdbe4a60-7692-4e9c-9df4-ad1711337c57">
  </img>  
</p>

<p align="center">
  OpenSpeedyは、フレームレート制限を突破し、よりスムーズで快適なゲーム加速体験を提供する、オープンソースかつ無料のゲーム速度調整ツールです。
</p>

<p align="center">
  <img src="https://api.visitorbadge.io/api/visitors?path=game1024.openspeedy&countColor=%234ecdc4">
  <br/>
    
  <a href="https://github.com/game1024/OpenSpeedy/stargazers">
    <img src="https://img.shields.io/github/stars/game1024/OpenSpeedy?style=for-the-badge&color=yellow" alt="GitHub Stars">
  </a>

  <img src="https://img.shields.io/github/forks/game1024/OpenSpeedy?style=for-the-badge&color=8a2be2" alt="GitHub Forks">

  <a href="https://github.com/game1024/OpenSpeedy/issues">
    <img src="https://img.shields.io/github/issues-raw/game1024/OpenSpeedy?style=for-the-badge&label=Issues&color=orange" alt="Github Issues">
  </a>
  <br/>  
  
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/downloads/game1024/OpenSpeedy/total?style=for-the-badge" alt="Downloads">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/v/release/game1024/OpenSpeedy?style=for-the-badge&color=brightgreen" alt="Version">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/actions">
      <img src="https://img.shields.io/github/actions/workflow/status/game1024/OpenSpeedy/ci.yml?style=for-the-badge" alt="Github Action">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy">
    <img src="https://img.shields.io/badge/Platform-Windows-lightblue?style=for-the-badge" alt="Platform">
  </a>
  <br/>
  
  <a href="https://github.com/game1024/OpenSpeedy/commits">
    <img src="https://img.shields.io/github/commit-activity/m/game1024/OpenSpeedy?style=for-the-badge" alt="コミットアクティビティ">
  </a>
  <img src="https://img.shields.io/badge/language-C/C++-blue?style=for-the-badge">
  <img src="https://img.shields.io/badge/License-GPLv3-green.svg?style=for-the-badge">
  <br/>
</p>

<p align="center">
  <a href="https://github.com/game1024/OpenSpeedy/blob/master/docs/README_cn.md">
    简体中文
  </a>
  ·
  <a href="https://github.com/game1024/OpenSpeedy/blob/master/docs/README_ja.md">
    日本語
  </a>
  ·
  <a href="https://github.com/game1024/OpenSpeedy?tab=readme-ov-file#-openspeedy-">
    English
  </a>
</p>

# 🚀 特徴
- 完全無料・オープンソース
- シンプルで使いやすいインターフェース
- 速度倍率をカスタマイズ可能
- 様々なゲームエンジンとの高い互換性
- 低いシステムリソース消費
- x86およびx64プラットフォームのプロセスを同時に加速可能
- カーネル非侵入型、Ring3レベルのHook、システムカーネルを破壊しない

# 📥 インストール
1. [インストールページ](https://github.com/game1024/OpenSpeedy/releases)にアクセスし、最新版をダウンロードしてください
2. ダウンロードしたファイルを任意の場所に解凍してください
3. インストール不要、「OpenSpeedy.exe」を直接実行するだけで利用できます

# 💻 動作環境
- OS: Windows10以降
- プラットフォーム：x86（32ビット）およびx64（64ビット）

# 📝 使い方
1. OpenSpeedyを起動する
2. 速度を変更したいゲームを起動する
<img src="https://github.com/user-attachments/assets/648e721d-9c3a-4d82-954c-19b16355d084" width="50%">

3. ゲームプロセスを選択し、OpenSpeedyの画面で速度倍率を調整する
<img src="https://github.com/user-attachments/assets/9469aae9-8be0-4e40-884d-1fbea3206e73" width="50%">

4. すぐに効果が反映されます。下記の比較動画をご覧ください

<video src="https://github.com/user-attachments/assets/74471b1f-7f95-4de8-b5aa-7edc85c9d5f0" width="70%"></video>

# 🔧 技術原理
OpenSpeedyは、以下のWindowsシステム時間関数をHookすることで、ゲーム速度を調整しています：

| 関数名 | 所属ライブラリ | 機能 |
|--------|--------|------------------|
| Sleep | user32.dll | スレッドのスリープ |
| SetTimer | user32.dll | メッセージベースのタイマー作成 |
| timeGetTime | winmm.dll | システム起動後の経過ミリ秒数取得 |
| GetTickCount | kernel32.dll | システム起動後の経過ミリ秒数取得 |
| GetTickCount64 | kernel32.dll | システム起動後の経過ミリ秒数取得 (64ビット) |
| QueryPerformanceCounter | kernel32.dll | 高精度パフォーマンスカウンタ |
| GetSystemTimeAsFileTime | kernel32.dll | システム時刻の取得 |
| GetSystemTimePreciseAsFileTime | kernel32.dll | 高精度なシステム時刻の取得 |

# ⚠️ 注意事項
- 本ツールは学習および研究用途のみを目的としています
- 一部オンラインゲームにはアンチチートシステムが搭載されています。本ツールの使用によりアカウントがBANされる場合があります
- 過度な加速は、ゲームの物理エンジン異常やクラッシュの原因となる場合があります
- 対戦型オンラインゲームでの利用は推奨しません
- オープンソース製品のためデジタル署名は付与しておらず、ウイルス対策ソフトによる誤検出の可能性があります

# 🔄 フィードバック
使用中に問題が発生した場合は、以下の方法でご連絡ください：
- [FAQ](https://github.com/game1024/OpenSpeedy/wiki#faq) - まずはWikiを確認して問題の場所を特定できます
- [GitHub Issues](https://github.com/game1024/OpenSpeedy/issues) - 不具合報告
- WeChat Group
<img width="30%" src="https://github.com/user-attachments/assets/d77cb35d-6c92-4480-89c9-40f124c4df08">

# 🎁 スポンサー
もしOpenSpeedyプロジェクトが役に立ったと感じたら、コーヒーをご馳走していただけると嬉しいです～ ☕️

365VPNは専用回線を利用して世界中を接続し、最大10Gbpsの速度を提供します。今すぐダウンロードして無料でサーフィンを始めましょう🏄: https://ref.365tz87989.com/?r=RWQVZD

# 📜 ライセンス
OpenSpeedyはGNU v3ライセンスに従っています。

# 🙏 クレジット
OpenSpeedyは以下のプロジェクトのソースコードを使用しています。オープンソースコミュニティに感謝します。OpenSpeedyが役立った場合はStarをお願いします！
- [minhook](https://github.com/TsudaKageyu/minhook): APIフック用
- [Qt](https://www.qt.io/): GUI

免責事項: OpenSpeedyは教育及び研究目的のみでご利用ください。本ソフトウェアの使用に伴うすべてのリスクと責任は利用者にあります。著者は本ソフトウェアの使用によるいかなる損害や法的責任も一切負いません。

<p align="center">
  <img src="https://api.star-history.com/svg?repos=game1024/openspeedy&type=Date" Alt="Star History Chart">
</p>
