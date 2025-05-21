#include "mainwindow.h"
#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <QLocale>
#include <QTranslator>
#include <windows.h>
#include <dbghelp.h>

// 链接dbghelp库
#pragma comment(lib, "dbghelp.lib")

typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    );

// 自定义异常处理函数
LONG WINAPI createMiniDump(EXCEPTION_POINTERS* exceptionPointers)
{
    // 创建dump目录
    QDir dumpDir(QDir::currentPath() + "/dumps");
    if (!dumpDir.exists()) {
        dumpDir.mkpath(".");
    }

    // 创建dump文件名
    QString dumpFileName = QString("%1/crash_%2.dmp")
                               .arg(dumpDir.absolutePath())
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    // 创建dump文件
    HANDLE hFile = CreateFile(
        dumpFileName.toStdWString().c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (hFile == INVALID_HANDLE_VALUE) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 设置dump信息
    MINIDUMP_EXCEPTION_INFORMATION exInfo;
    exInfo.ThreadId = GetCurrentThreadId();
    exInfo.ExceptionPointers = exceptionPointers;
    exInfo.ClientPointers = FALSE;

    // 设置dump类型 - 可以根据需要修改
    MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
        MiniDumpWithFullMemory |
        MiniDumpWithHandleData |
        MiniDumpWithThreadInfo |
        MiniDumpWithUnloadedModules
        );

    // 写入dump文件
    BOOL success = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        dumpType,
        &exInfo,
        NULL,
        NULL
        );

    CloseHandle(hFile);

    // 通知用户
    if (success) {
        QMessageBox::information(nullptr, "应用崩溃",
                                 QString("应用程序发生崩溃，已生成转储文件：%1").arg(dumpFileName));
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    SetUnhandledExceptionFilter(createMiniDump);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);

    // 检查是否已有实例在运行
    QString unique = "OpenSpeedy";
    QLocalSocket socket;
    socket.connectToServer(unique);
    if (socket.waitForConnected(500)) {
        socket.close();
        return -1;
    }

    // 使用资源文件中的图标
    QIcon appIcon;
    appIcon.addFile(":/icons/images/icon_16.ico", QSize(16,16));
    appIcon.addFile(":/icons/images/icon_32.ico", QSize(32,32));
    appIcon.addFile(":/icons/images/icon_64.ico", QSize(64,64));
    a.setWindowIcon(appIcon);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "OpenSpeedy_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.resize(960, 640);
    w.show();

    // 创建并启动本地服务器
    QLocalServer server;
    QLocalServer::removeServer(unique);
    server.listen(unique);
    // 当用户尝试再运行一个进程时，将窗口显示到最前台
    QObject::connect(&server, &QLocalServer::newConnection, [&]{
        w.show();
        w.raise();
        w.showNormal();
        w.activateWindow();
    });
    return a.exec();
}
