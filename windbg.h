/*
 * OpenSpeedy - Open Source Game Speed Controller
 * Copyright (C) 2025 Game1024
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef WINDBG_H
#define WINDBG_H
#include <windows.h>
#include <QDateTime>
#include <QDir>
#include <dbghelp.h>
// 链接dbghelp库
#pragma comment(lib, "dbghelp.lib")

typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

// 自定义异常处理函数
LONG WINAPI createMiniDump(EXCEPTION_POINTERS *exceptionPointers)
{
    // 创建dump文件目录
    QDir dumpDir(QDir::currentPath() + "/dumps");
    if (!dumpDir.exists())
    {
        dumpDir.mkpath(".");
    }

    // 生成带时间戳的文件名
    QString dumpFileName =
        QString("%1/crash_%2.dmp")
            .arg(dumpDir.absolutePath())
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    // 创建文件
    HANDLE hFile = CreateFile(dumpFileName.toStdWString().c_str(),
                              GENERIC_WRITE, FILE_SHARE_READ, NULL,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 加载dbghelp.dll
    HMODULE dbgHelp = LoadLibrary(L"dbghelp.dll");
    if (!dbgHelp)
    {
        CloseHandle(hFile);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 获取MiniDumpWriteDump函数地址
    MINIDUMPWRITEDUMP miniDumpWriteDump =
        (MINIDUMPWRITEDUMP)GetProcAddress(dbgHelp, "MiniDumpWriteDump");

    if (!miniDumpWriteDump)
    {
        FreeLibrary(dbgHelp);
        CloseHandle(hFile);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 配置异常信息
    MINIDUMP_EXCEPTION_INFORMATION exInfo;
    exInfo.ThreadId = GetCurrentThreadId();
    exInfo.ExceptionPointers = exceptionPointers;
    exInfo.ClientPointers = FALSE;

    // 设置dump类型
    MINIDUMP_TYPE dumpType =
        (MINIDUMP_TYPE)(MiniDumpWithFullMemory |      // 包含完整内存
                        MiniDumpWithFullMemoryInfo |  // 包含内存信息
                        MiniDumpWithHandleData |      // 包含句柄数据
                        MiniDumpWithThreadInfo |      // 包含线程信息
                        MiniDumpWithUnloadedModules   // 包含卸载的模块
        );

    // 写入dump文件
    BOOL success = miniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                                     hFile, dumpType, &exInfo, NULL, NULL);

    // 清理资源
    FreeLibrary(dbgHelp);
    CloseHandle(hFile);

    // 显示通知
    if (success)
    {
        std::wstring message =
            QString("程序遇到错误已退出，崩溃转储文件已保存到：\n%1")
                .arg(dumpFileName)
                .toStdWString();
        MessageBoxW(nullptr, message.c_str(), L"程序崩溃",
                    MB_OK | MB_ICONERROR);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif  // WINDBG_H
