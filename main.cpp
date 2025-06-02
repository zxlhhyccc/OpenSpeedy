#include "mainwindow.h"
#include "mml.h"
#include "windbg.h"
#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLocale>
#include <QTranslator>
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
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "OpenSpeedy_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.resize(800, 640);
    w.show();

    // 创建并启动本地服务器
    QLocalServer server;
    QLocalServer::removeServer(unique);
    server.listen(unique);
    // 当用户尝试再运行一个进程时，将窗口显示到最前台
    QObject::connect(&server, &QLocalServer::newConnection,
                     [&]
                     {
                         w.show();
                         w.raise();
                         w.showNormal();
                         w.activateWindow();
                     });
    return a.exec();
}
