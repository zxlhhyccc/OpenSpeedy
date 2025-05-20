#include "winutils.h"

#include <psapi.h>
#include <tlhelp32.h>
#include <QDebug>
#include <QFileInfo>
#include <string>
#include <wchar.h>

static QSet<std::wstring> systemNames = {
    L"svchost.exe",
    L"wininit.exe",
    L"RuntimeBroker.exe",
};

winutils::winutils() {}

bool winutils::injectDll(DWORD processId, const std::wstring &dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        qDebug() << "Failed to open process:" << GetLastError();
        return false;
    }

    if (checkDllExist(processId, dllPath)) {
        qDebug() << "Process already have been injected";
        return true;
    }

    void* pRemoteMemory = VirtualAllocEx(hProcess, nullptr, dllPath.size() * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        qDebug() << "Failed to allocate memory in target process:" << GetLastError();
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), dllPath.size() * sizeof(wchar_t), nullptr)) {
        qDebug() << "Failed to write memory in target process:" << GetLastError();
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    if (!hKernel32) {
        qDebug() << "Failed to get handle for kernel32.dll:" << GetLastError();
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC loadLibraryAddr = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!loadLibraryAddr) {
        qDebug() << "Failed to get address of LoadLibraryW:" << GetLastError();
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, pRemoteMemory, 0, nullptr);
    if (!hThread) {
        qDebug() << "Failed to create remote thread:" << GetLastError();
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

bool winutils::unhookDll(DWORD processId, const std::wstring &dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        qDebug() << "Failed to open process:" << GetLastError();
        return false;
    }

    HMODULE hModules[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t moduleName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hModules[i], moduleName, sizeof(moduleName) / sizeof(wchar_t))) {
                if (dllPath == moduleName) {
                    FARPROC freeLibraryAddr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
                    if (!freeLibraryAddr) {
                        qDebug() << "Failed to get address of FreeLibrary:" << GetLastError();
                        CloseHandle(hProcess);
                        return false;
                    }

                    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)freeLibraryAddr, hModules[i], 0, nullptr);
                    if (!hThread) {
                        qDebug() << "Failed to create remote thread for FreeLibrary:" << GetLastError();
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

bool winutils::checkDllExist(DWORD processId, const std::wstring &dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        qDebug() << "打开进程失败:" << GetLastError();
        return false;
    }

    bool dllFound = false;
    HMODULE hModules[1024];
    DWORD cbNeeded;

    // 获取进程中所有已加载的模块
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        DWORD moduleCount = cbNeeded / sizeof(HMODULE);
        for (DWORD i = 0; i < moduleCount; i++) {
            wchar_t moduleName[MAX_PATH] = {0};

            // 获取模块的完整路径
            if (GetModuleFileNameEx(hProcess, hModules[i], moduleName, sizeof(moduleName) / sizeof(wchar_t))) {
                // 比较模块路径和指定的DLL路径
                if (dllPath == std::wstring(moduleName)) {
                    dllFound = true;
                    break;
                }
            }
        }
    } else {
        qDebug() << "枚举进程模块失败:" << GetLastError();
    }

    CloseHandle(hProcess);
    return dllFound;
}

QList<ProcessInfo> winutils::getProcessList() {
    QList<ProcessInfo> processList;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        qDebug() << "创建进程快照失败: " << GetLastError();
        return processList;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        qDebug() << "获取首个进程信息失败: " << GetLastError();
        CloseHandle(hProcessSnap);
        return processList;
    }

    do {
        if (systemNames.contains(pe32.szExeFile)) {
            continue;
        }
        ProcessInfo info;
        info.pid = pe32.th32ProcessID;
        info.parentPid = pe32.th32ParentProcessID;
        info.name = QString::fromWCharArray(pe32.szExeFile);
        info.threadCount = pe32.cntThreads;

        // 获取内存使用和优先级信息
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
        if (hProcess) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                info.memoryUsage = pmc.WorkingSetSize;
            } else {
                info.memoryUsage = 0;
            }
            BOOL wow64Process = FALSE;
            IsWow64Process(hProcess, &wow64Process);
            info.is64Bit = !wow64Process;
            info.priorityClass = GetPriorityClass(hProcess);
            CloseHandle(hProcess);
        } else {
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
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

    if (hProcess != NULL) {
        wchar_t buffer[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, NULL, buffer, MAX_PATH) > 0) {
            processPath = QString::fromWCharArray(buffer);
        }
        CloseHandle(hProcess);
    }

    return processPath;
}

