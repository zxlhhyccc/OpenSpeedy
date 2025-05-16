#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
    w.show();
    return a.exec();
}
