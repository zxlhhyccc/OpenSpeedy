#include "winutils.h"
#include <QDebug>
#include <QFileInfo>
#include <psapi.h>
#include <string>
#include <tlhelp32.h>
#include <wchar.h>
#include <winternl.h>

static QSet<std::wstring> systemNames = {
    L"svchost.exe",
    L"wininit.exe",
    L"RuntimeBroker.exe",
};

winutils::winutils() {}

bool winutils::injectDll(DWORD processId, const std::wstring &dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess)
    {
        qDebug() << "Failed to open process:" << GetLastError();
        return false;
    }

    if (checkDllExist(processId, dllPath))
    {
        qDebug() << "Process already have been injected";
        return true;
    }

    if (injectDllViaCRT(processId, dllPath))
    {
        return true;
    }
    else if (injectDllViaAPC(processId, dllPath))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool winutils::injectDllViaCRT(DWORD processId, const std::wstring &dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess)
    {
        qDebug() << "Failed to open process:" << GetLastError();
        return false;
    }

    if (checkDllExist(processId, dllPath))
    {
        qDebug() << "Process already have been injected";
        return true;
    }

    void *pDllPath = VirtualAllocEx(
        hProcess, nullptr, (dllPath.size() + 1) * sizeof(wchar_t),
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!pDllPath)
    {
        qDebug() << "Failed to allocate memory in target process:"
                 << GetLastError();
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(),
                            (dllPath.size() + 1) * sizeof(wchar_t), nullptr))
    {
        qDebug() << "Failed to write memory in target process:"
                 << GetLastError();
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    if (!hKernel32)
    {
        qDebug() << "Failed to get handle for kernel32.dll:" << GetLastError();
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibraryW)
    {
        qDebug() << "Failed to get address of LoadLibraryW:" << GetLastError();
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
                                        (LPTHREAD_START_ROUTINE)pLoadLibraryW,
                                        pDllPath, 0, nullptr);
    if (!hThread)
    {
        qDebug() << "Failed to create remote thread:" << GetLastError();
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    // 检查LoadLibrary是否执行成功
    DWORD exitCode = 0;
    if (GetExitCodeThread(hThread, &exitCode))
    {
        qDebug() << "Remote thread exit code:" << exitCode;
        // LoadLibrary返回的是模块句柄，如果为0则失败
        if (exitCode == 0)
        {
            VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            CloseHandle(hThread);
            qDebug() << "LoadLibrary failed in remote process";
            return false;
        }
    }

    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

bool winutils::injectDllViaAPC(DWORD processId, const std::wstring &dllPath)
{
    HANDLE hProcess =
        OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ |
                        PROCESS_QUERY_INFORMATION,
                    FALSE, processId);

    if (!hProcess)
    {
        qDebug() << "Failed to open process: " << GetLastError();
        return false;
    }

    if (checkDllExist(processId, dllPath))
    {
        qDebug() << "Process already have been injected";
        return true;
    }

    // 在目标进程中分配内存并写入DLL路径
    SIZE_T pathSize = (dllPath.length() + 1) * sizeof(WCHAR);
    LPVOID pDllPath =
        VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);

    if (!pDllPath)
    {
        qDebug() << "Failed to allocate memory: " << GetLastError();
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(), pathSize,
                            NULL))
    {
        qDebug() << "Failed to write memory: " << GetLastError();
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 获取LoadLibraryW的地址
    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    if (!hKernel32)
    {
        qDebug() << "Failed to get kernel32 handle";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibraryW)
    {
        qDebug() << "Failed to get LoadLibraryW address";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 获取进程中的所有线程
    DWORD threadId = getProcessMainThread(processId);
    if (!threadId)
    {
        qDebug() << "No threads found in the process";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 向目标进程的所有线程队列中添加APC调用
    bool success = false;

    // 打开线程
    HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, threadId);
    if (hThread)
    {
        // 将LoadLibraryW作为APC函数加入队列
        PostThreadMessageW(threadId, WM_PAINT, 0, 0);
        DWORD result =
            QueueUserAPC((PAPCFUNC)pLoadLibraryW, hThread, (ULONG_PTR)pDllPath);
        PostThreadMessageW(threadId, WM_PAINT, 0, 0);
        if (result != 0)
        {
            success = true;
            qDebug() << "APC queued to thread " << threadId;
        }

        CloseHandle(hThread);
    }

    if (!success)
    {
        qDebug() << "Failed to queue APC to any thread";
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 注意：我们不释放分配的内存，因为APC函数尚未执行
    // 内存将在DLL加载后释放

    CloseHandle(hProcess);
    return true;
}

bool winutils::unhookDll(DWORD processId, const std::wstring &dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess)
    {
        qDebug() << "Failed to open process:" << GetLastError();
        return false;
    }

    HMODULE hModules[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            wchar_t moduleName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hModules[i], moduleName,
                                    sizeof(moduleName) / sizeof(wchar_t)))
            {
                if (dllPath == moduleName)
                {
                    FARPROC pFreeLibrary = GetProcAddress(
                        GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
                    if (!pFreeLibrary)
                    {
                        qDebug() << "Failed to get address of FreeLibrary:"
                                 << GetLastError();
                        CloseHandle(hProcess);
                        return false;
                    }

                    HANDLE hThread =
                        CreateRemoteThread(hProcess, nullptr, 0,
                                           (LPTHREAD_START_ROUTINE)pFreeLibrary,
                                           hModules[i], 0, nullptr);
                    if (!hThread)
                    {
                        qDebug()
                            << "Failed to create remote thread for FreeLibrary:"
                            << GetLastError();
                        CloseHandle(hProcess);
                        return false;
                    }

                    WaitForSingleObject(hThread, INFINITE);
                    CloseHandle(hThread);
                    CloseHandle(hProcess);
                    return true;
                }
            }
        }
    }

    CloseHandle(hProcess);
    return false;
}

bool winutils::checkDllExist(DWORD processId, const std::wstring &dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE, processId);
    if (!hProcess)
    {
        qDebug() << "打开进程失败:" << GetLastError();
        return false;
    }

    bool dllFound = false;
    HMODULE hModules[1024];
    DWORD cbNeeded;

    // 获取进程中所有已加载的模块
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded))
    {
        DWORD moduleCount = cbNeeded / sizeof(HMODULE);
        for (DWORD i = 0; i < moduleCount; i++)
        {
            WCHAR moduleName[MAX_PATH] = {0};

            // 获取模块的完整路径
            if (GetModuleFileNameExW(hProcess, hModules[i], moduleName,
                                     sizeof(moduleName) / sizeof(WCHAR)))
            {
                // 比较模块路径和指定的DLL路径
                if (dllPath == std::wstring(moduleName))
                {
                    dllFound = true;
                    break;
                }
            }
        }
    }
    else
    {
        qDebug() << "枚举进程模块失败:" << GetLastError();
    }

    CloseHandle(hProcess);
    return dllFound;
}

typedef struct _OSVERSIONINFOEXW RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;

typedef LONG NTSTATUS;
typedef NTSTATUS(WINAPI *fnRtlGetVersion)(PRTL_OSVERSIONINFOEXW);

BOOL winutils::getWindowsVersion(DWORD *majorVersion,
                                 DWORD *minorVersion,
                                 DWORD *buildNumber)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return FALSE;

    fnRtlGetVersion RtlGetVersion =
        (fnRtlGetVersion)GetProcAddress(hNtdll, "RtlGetVersion");
    if (!RtlGetVersion) return FALSE;

    RTL_OSVERSIONINFOEXW osInfo;
    ZeroMemory(&osInfo, sizeof(RTL_OSVERSIONINFOEXW));
    osInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    NTSTATUS status = RtlGetVersion(&osInfo);
    if (status != 0)  // STATUS_SUCCESS = 0
        return FALSE;

    if (majorVersion) *majorVersion = osInfo.dwMajorVersion;
    if (minorVersion) *minorVersion = osInfo.dwMinorVersion;
    if (buildNumber) *buildNumber = osInfo.dwBuildNumber;

    return TRUE;
}

QString winutils::getWindowsVersion()
{
    static char version[100] = {0};
    DWORD major, minor, build;

    if (!getWindowsVersion(&major, &minor, &build))
        return "Unknown Windows Version";

    // 确定Windows版本
    if (major == 10)
    {
        if (build >= 22000)
        {
            sprintf(version, "Windows 11 (Build %lu)", build);
        }
        else
        {
            sprintf(version, "Windows 10 (Build %lu)", build);
        }
    }
    else if (major == 6)
    {
        switch (minor)
        {
            case 3:
                sprintf(version, "Windows 8.1 (Build %lu)", build);
                break;
            case 2:
                sprintf(version, "Windows 8 (Build %lu)", build);
                break;
            case 1:
                sprintf(version, "Windows 7 (Build %lu)", build);
                break;
            case 0:
                sprintf(version, "Windows Vista (Build %lu)", build);
                break;
            default:
                sprintf(version, "Windows NT %lu.%lu (Build %lu)", major, minor,
                        build);
        }
    }
    else if (major == 5)
    {
        switch (minor)
        {
            case 2:
                sprintf(version, "Windows Server 2003 (Build %lu)", build);
                break;
            case 1:
                sprintf(version, "Windows XP (Build %lu)", build);
                break;
            case 0:
                sprintf(version, "Windows 2000 (Build %lu)", build);
                break;
            default:
                sprintf(version, "Windows NT %lu.%lu (Build %lu)", major, minor,
                        build);
        }
    }
    else
    {
        sprintf(version, "Windows NT %lu.%lu (Build %lu)", major, minor, build);
    }
    return version;
}

QList<ProcessInfo> winutils::getProcessList()
{
    QList<ProcessInfo> processList;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        qDebug() << "创建进程快照失败: " << GetLastError();
        return processList;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        qDebug() << "获取首个进程信息失败: " << GetLastError();
        CloseHandle(hProcessSnap);
        return processList;
    }

    do
    {
        if (systemNames.contains(pe32.szExeFile))
        {
            continue;
        }
        ProcessInfo info;
        info.pid = pe32.th32ProcessID;
        info.parentPid = pe32.th32ParentProcessID;
        info.name = QString::fromWCharArray(pe32.szExeFile);
        info.threadCount = pe32.cntThreads;

        // 获取内存使用和优先级信息
        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
        if (hProcess)
        {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                info.memoryUsage = pmc.WorkingSetSize;
            }
            else
            {
                info.memoryUsage = 0;
            }
            BOOL wow64Process = FALSE;
            IsWow64Process(hProcess, &wow64Process);
            info.is64Bit = !wow64Process;
            info.priorityClass = GetPriorityClass(hProcess);
            CloseHandle(hProcess);
        }
        else
        {
            info.memoryUsage = 0;
            info.priorityClass = 0;
        }

        processList.append(info);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return processList;
}

QString winutils::getProcessPath(DWORD processId)
{
    QString processPath;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE, processId);

    if (hProcess != NULL)
    {
        wchar_t buffer[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, NULL, buffer, MAX_PATH) > 0)
        {
            processPath = QString::fromWCharArray(buffer);
        }
        CloseHandle(hProcess);
    }

    return processPath;
}

DWORD winutils::getProcessMainThread(DWORD processId)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        qDebug() << "Failed to create thread snapshot:" << GetLastError();
        return 0;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    DWORD mainThreadId = 0;
    FILETIME earliestTime = {0xFFFFFFFF, 0xFFFFFFFF};  // 设为最大值

    if (Thread32First(hSnapshot, &te32))
    {
        do
        {
            if (te32.th32OwnerProcessID == processId)
            {
                // 打开线程获取创建时间
                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE,
                                            te32.th32ThreadID);
                if (hThread)
                {
                    FILETIME creationTime, exitTime, kernelTime, userTime;
                    if (GetThreadTimes(hThread, &creationTime, &exitTime,
                                       &kernelTime, &userTime))
                    {
                        // 比较创建时间，找最早的线程
                        if (CompareFileTime(&creationTime, &earliestTime) < 0)
                        {
                            earliestTime = creationTime;
                            mainThreadId = te32.th32ThreadID;
                        }
                    }
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnapshot, &te32));
    }

    CloseHandle(hSnapshot);
    return mainThreadId;
}

bool winutils::enableAllPrivilege()
{
    HANDLE hToken;
    // 获取进程token
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

    const wchar_t *essentialPrivileges[] = {
        SE_DEBUG_NAME,               // 最重要的调试权限
        SE_INCREASE_QUOTA_NAME,      // 内存配额
        SE_PROF_SINGLE_PROCESS_NAME  // 进程分析
    };

    TOKEN_PRIVILEGES tkp;
    bool success = true;

    for (const auto &privilege : essentialPrivileges)
    {
        if (LookupPrivilegeValue(NULL, privilege, &tkp.Privileges[0].Luid))
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, NULL);
        }
    }

    CloseHandle(hToken);
    return success;
}
