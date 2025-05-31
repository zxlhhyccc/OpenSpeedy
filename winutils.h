#ifndef WINUTILS_H
#define WINUTILS_H
#include <windows.h>
#include <QSet>
#include <QString>
#include <string>
#include <tlhelp32.h>

struct ProcessInfo
{
    DWORD pid;
    DWORD parentPid;
    QString name;
    DWORD threadCount;
    bool is64Bit;
    DWORD priorityClass;
    SIZE_T memoryUsage;
};

class winutils
{
   public:
    winutils();

   public:
    // DLL 注入
    static bool injectDll(DWORD processId, const std::wstring& dllPath);

    // 远程 DLL 注入
    static bool injectDllViaCRT(DWORD processId, const std::wstring& dllPath);

    // APC DLL 注入
    static bool injectDllViaAPC(DWORD processId, const std::wstring& dllPath);

    // DLL 卸载
    static bool unhookDll(DWORD processId, const std::wstring& dllPath);

    // 检查DLL是否已挂载
    static bool checkDllExist(DWORD processId, const std::wstring& dllPath);

    static BOOL getWindowsVersion(DWORD* majorVersion,
                                  DWORD* minorVersion,
                                  DWORD* buildNumber);

    static QString getWindowsVersion();

    // 获取进程快照
    static QList<ProcessInfo> getProcessList();

    // 获取进程路径
    static QString getProcessPath(DWORD processId);

    // 获取进程中的所有线程
    static DWORD getProcessMainThread(DWORD processId);

    static bool enableAllPrivilege();
};

#endif  // WINUTILS_H
