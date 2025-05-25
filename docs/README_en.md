<h1 align="center"> OpenSpeedy </h1>

<p align="center">
  <img style="margin:0 auto" width=100 height=100 src="https://github.com/user-attachments/assets/bdbe4a60-7692-4e9c-9df4-ad1711337c57">
  </img>  
</p>

<p align="center">
  OpenSpeedy is an open-source and free game speed modification tool that helps break frame rate limits, providing a smoother gaming experience.
</p>

<p align="center">
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/v/release/game1024/OpenSpeedy?color=brightgreen" alt="Version">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/releases">
    <img src="https://img.shields.io/github/downloads/game1024/OpenSpeedy/total" alt="Downloads">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy">
    <img src="https://img.shields.io/badge/Platform-Windows-lightblue" alt="Platform">
  </a>
  <a href="https://github.com/game1024/OpenSpeedy/commits">
    <img src="https://img.shields.io/github/commit-activity/m/game1024/OpenSpeedy" alt="Commit Activity">
  </a>

  <img src="https://img.shields.io/badge/language-C/C++-blue">
  <img src="https://img.shields.io/badge/License-GPLv3-green.svg">
  <a href="https://github.com/game1024/OpenSpeedy/stargazers">
    <img src="https://img.shields.io/github/stars/game1024/OpenSpeedy" alt="GitHub Stars">
  </a>
</p>

# 🚀 Features
- Completely free and open-source
- Simple and user-friendly interface
- Customizable speed adjustment
- Compatible with various game engines
- Low system resource usage
- Can accelerate both x86 and x64 platform processes
- No kernel intrusion, Ring3 layer Hook, doesn't compromise system kernel integrity

# 📥 Installation
1. Visit the [release page](https://github.com/game1024/OpenSpeedy/releases) to download the latest version
2. Extract the downloaded file to any location
3. No installation required, simply run OpenSpeedy.exe to use

# 💻 System Requirements
- OS: Windows 10 or higher
- Platform: x86 (32-bit) and x64 (64-bit)

# 📝 Usage Instructions
1. Launch OpenSpeedy
2. Run the target game you want to modify
<img src="https://github.com/user-attachments/assets/648e721d-9c3a-4d82-954c-19b16355d084" width="50%">

3. Select the game process and adjust the speed multiplier in the OpenSpeedy interface
<img src="https://github.com/user-attachments/assets/73ef1a95-64fb-4017-939c-a44405a6e742" width="50%">

4. Changes take effect immediately. Comparison shown below:

<video src="https://github.com/user-attachments/assets/74471b1f-7f95-4de8-b5aa-7edc85c9d5f0" width="70%"></video>

# 🔧 Technical Principles
OpenSpeedy works by hooking the following Windows system time functions:

|Function Name	| Library |	Purpose |
|--------|----------|------------------|
|Sleep|user32.dll|Thread sleep|
|SetTimer|user32.dll|Creates a message-based timer|
|timeGetTime | winmm.dll	| Retrieves milliseconds elapsed since system startup |
|GetTickCount | kernel32.dll	| Retrieves milliseconds elapsed since system startup |
|GetTickCount64	| kernel32.dll	| Retrieves milliseconds elapsed since system startup (64-bit) |
|QueryPerformanceCounter |	kernel32.dll	| High-precision performance counter |
|GetSystemTimeAsFileTime |	kernel32.dll	| Retrieves system time |
|GetSystemTimePreciseAsFileTime |	kernel32.dll	| Retrieves high-precision system time |

# ⚠️ Precautions
- This tool is for educational and research purposes only
- Some online games may have anti-cheat systems, using this tool might result in account bans
- Excessive acceleration may cause game physics engine anomalies or crashes
- Not recommended for use in competitive online games

# 🔄 Feedback
If you encounter any issues while using OpenSpeedy, please provide feedback through:
- [GitHub Issues](https://github.com/game1024/OpenSpeedy/issues) - Submit problem reports

# 📜 License
OpenSpeedy is released under the GNU GPLv3 License.

# 🙏 Acknowledgements
OpenSpeedy utilizes source code from the following projects. We thank the open-source community for their contributions:
- [minhook](https://github.com/TsudaKageyu/minhook): Used for API Hooking
- [Qt](https://www.qt.io/): GUI Framework

Disclaimer: OpenSpeedy is for educational and research purposes only. Users assume all risks and responsibilities associated with using this software. The author is not responsible for any losses or legal liabilities arising from the use of this software.

<p align="center">
  <img src="https://api.star-history.com/svg?repos=game1024/openspeedy&type=Date" Alt="Star History Chart">
</p>
