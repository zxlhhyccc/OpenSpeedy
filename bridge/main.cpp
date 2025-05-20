#include <QCoreApplication>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QDir>
#include "../speedpatch/speedpatch.h"
#include "../config.h"
#include "../winutils.h"

#ifndef _WIN64
    #define SPEEDPATCH_DLL SPEEDPATCH32_DLL
#else
    #define SPEEDPATCH_DLL SPEEDPATCH64_DLL
#endif

void handleInject(int processId, QString dllPath) {
    qDebug() << "执行 inject，进程ID:" << processId;
    // 这里实现你的inject逻辑
    winutils::injectDll(processId, dllPath.toStdWString());

}

void handleUnhook(int processId, QString dllPath) {
    qDebug() << "执行 unhook，进程ID:" << processId;
    // 这里实现你的unhook逻辑
    winutils::unhookDll(processId, dllPath.toStdWString());
}

void handleChange(double factor) {
    qDebug() << "执行 change，参数:" << factor;
    // 这里实现你的change逻辑
    ChangeSpeed(factor);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString dllPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath()+ "/" + SPEEDPATCH_DLL);

    QTextStream in(stdin);
    QTextStream out(stdout);

    QRegularExpression injectRegex("^\\s*inject\\s+(\\d+)\\s*$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression unhookRegex("^\\s*unhook\\s+(\\d+)\\s*$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression changeRegex("^\\s*change\\s+([+-]?\\d*\\.?\\d+)\\s*$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression exitRegex("^\\s*exit\\s*$", QRegularExpression::CaseInsensitiveOption);

    while (true) {
        QString line = in.readLine();
        if (line.isNull()) {
            // 管道关闭或输入结束
            break;
        }
        line = line.trimmed();
        if (line.isEmpty()) continue;

        QRegularExpressionMatch match;
        if ((match = injectRegex.match(line)).hasMatch()) {
            int pid = match.captured(1).toInt();
            handleInject(pid, dllPath);
        } else if ((match = unhookRegex.match(line)).hasMatch()) {
            int pid = match.captured(1).toInt();
            handleUnhook(pid, dllPath);
        } else if ((match = changeRegex.match(line)).hasMatch()) {
            double factor = match.captured(1).toDouble();
            handleChange(factor);
        } else if (exitRegex.match(line).hasMatch()) {
            qDebug() << "收到exit命令，程序即将退出。";
            break;
        } else {
            qDebug() << "无效命令：" << line;
        }
    }

    return 0;
}
