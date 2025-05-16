#ifndef WINUTILS_H
#define WINUTILS_H

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <QString>
#include <QIcon>
#include <QSet>

struct ProcessInfo {
    DWORD pid;
    DWORD parentPid;
    QString name;
    DWORD threadCount;
    DWORD priorityClass;
    SIZE_T memoryUsage;
};

class winutils
{
public:
    winutils();

public:
    // DLL 注入
    static bool injectDll(DWORD processId, const std::wstring &dllPath);

    // DLL 卸载
    static bool unhookDll(DWORD processId, const std::wstring &dllPath);

    // 检查DLL是否已挂载
    static bool checkDllExist(DWORD processId, const std::wstring &dllPath);

    // 获取进程快照
    static QList<ProcessInfo> getProcessList();

    static QString getProcessPath(DWORD processId);

    // 获取进程图标
    static QIcon getProcessIcon(QString processPath);

    static QIcon getDefaultIcon(const QString &processName);

private:
};

#endif // WINUTILS_H
