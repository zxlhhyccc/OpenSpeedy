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
#include "mainwindow.h"
#include "windbg.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLocale>
#include <QTranslator>
#include <ShellScalingApi.h>
int
main(int argc, char* argv[])
{
    SetUnhandledExceptionFilter(createMiniDump);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QApplication a(argc, argv);
    winutils::enableAllPrivilege();

    // 检查是否已有实例在运行
    QString unique = "OpenSpeedy";
    QLocalSocket socket;
    socket.connectToServer(unique);
    if (socket.waitForConnected(500))
    {
        socket.close();
        return -1;
    }

    // 使用资源文件中的图标
    QIcon appIcon;
    appIcon.addFile(":/icons/images/icon_16.ico", QSize(16, 16));
    appIcon.addFile(":/icons/images/icon_32.ico", QSize(32, 32));
    appIcon.addFile(":/icons/images/icon_64.ico", QSize(64, 64));
    a.setWindowIcon(appIcon);

    QSettings settings =
      QSettings(QCoreApplication::applicationDirPath() + "/config.ini",
                QSettings::IniFormat);

    QTranslator translator;
    const QString baseName =
      "OpenSpeedy_" +
      settings.value(CONFIG_LANGUAGE, QLocale().system().name()).toString();

    if (translator.load(":/i18n/translations/" + baseName))
    {
        a.installTranslator(&translator);
    }

    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("OpenSpeedy");
    QCommandLineOption minimizeOption(
      QStringList() << "m" << "minimize-to-tray", "启动时最小化到托盘");
    parser.addOption(minimizeOption);
    parser.process(a);

    MainWindow w;
    w.resize(1024, 768);

    if (parser.isSet(minimizeOption))
    {
        w.hide();
    }
    else
    {
        w.show();
    }

    // 创建并启动本地服务器
    QLocalServer server;
    QLocalServer::removeServer(unique);
    server.listen(unique);
    // 当用户尝试再运行一个进程时，将窗口显示到最前台
    QObject::connect(&server,
                     &QLocalServer::newConnection,
                     [&]
                     {
                         w.show();
                         w.raise();
                         w.showNormal();
                         w.activateWindow();
                     });
    return a.exec();
}
