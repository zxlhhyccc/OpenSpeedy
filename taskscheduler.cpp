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
#include "taskscheduler.h"
#include <QDebug>
#include <QDir>
#include <QProcess>
TaskScheduler::TaskScheduler(QObject *parent) : QObject{parent}
{
}

TaskScheduler::~TaskScheduler()
{
}

bool TaskScheduler::createStartupTask(const QString &taskName,
                                      const QString &executablePath)
{
    if (taskName.isEmpty() || executablePath.isEmpty())
    {
        qWarning() << "任务名称或可执行文件路径不能为空";
        return false;
    }

    if (!QFile::exists(executablePath))
    {
        qWarning() << "可执行文件不存在" << executablePath;
        return false;
    }

    QStringList arguments;
    arguments << "/create"
              << "/f"
              << "/tn" << taskName << "/tr"
              << QString("\"%1\" --minimize-to-tray").arg(executablePath)
              << "/sc" << "onlogon"
              << "/delay" << "0000:10"
              << "/rl" << "highest";

    return execute(arguments);
}

bool TaskScheduler::deleteTask(const QString &taskName)
{
    if (taskName.isEmpty())
    {
        qWarning() << "任务名称不能为空";
        return false;
    }

    if (!isTaskExists(taskName))
    {
        qWarning() << "任务不存在, 无需删除" << taskName;
        return true;
    }

    QStringList arguments;
    arguments << "/delete"
              << "/f"
              << "/tn" << taskName;

    return execute(arguments);
}

bool TaskScheduler::enableTask(const QString &taskName, bool enable)
{
    if (!isTaskExists(taskName))
    {
        qWarning() << "任务不存在:" << taskName;
        return false;
    }

    QStringList arguments;
    arguments << "/change"
              << "/tn" << taskName << (enable ? "/enable" : "/disable");
    return execute(arguments);
}

bool TaskScheduler::isTaskExists(const QString &taskName)
{
    QStringList arguments;
    arguments << "/query" << "/tn" << taskName;

    return execute(arguments);
}

bool TaskScheduler::execute(const QStringList &arguments)
{
    QProcess process;
    process.setProgram("schtasks");
    process.setArguments(arguments);

    const int timeout = 10000;
    process.start();
    if (!process.waitForStarted(timeout))
    {
        qWarning() << "无法运行:" << process.errorString();
        return false;
    }

    if (!process.waitForFinished(timeout))
    {
        qWarning() << "执行超时";
        process.kill();
        return false;
    }

    int exitcode = process.exitCode();
    QString stdout_ = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString stderr_ = QString::fromLocal8Bit(process.readAllStandardError());

    if (exitcode == 0)
    {
        qDebug() << "执行成功:" << arguments;
        qDebug() << "输出:" << stdout_;
        return true;
    }
    else
    {
        qDebug() << "执行失败:" << arguments;
        qDebug() << "错误输出:" << stderr_;
        qDebug() << "标准输出:" << stdout_;
        return false;
    }
}

bool TaskScheduler::createStartupShortcut(const QString &taskName,
                                          const QString &executablePath)
{
    QString home = QDir::homePath();
    QString startupDir =
        home +
        "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
    QString shortcutPath = QDir(startupDir).absoluteFilePath(taskName + ".lnk");
    QString arguments = "--minimize-to-tray";

    QString psScript =
        QString("$WScriptShell = New-Object -ComObject WScript.Shell; "
                "$Shortcut = $WScriptShell.CreateShortcut('%1'); "
                "$Shortcut.TargetPath = '%2'; "
                "$Shortcut.Arguments = '%3'; "
                "$Shortcut.Description = '%4'; "
                "$Shortcut.Save()")
        .arg(shortcutPath, executablePath, arguments,
             QString("启动 %1").arg(QFileInfo(executablePath).baseName()));

    return executePs(psScript);
}

bool TaskScheduler::deleteStartupShortcut(const QString &taskName)
{
    QString home = QDir::homePath();
    QString startupDir =
        home + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
    QString shortcutPath = QDir(startupDir).absoluteFilePath(taskName + ".lnk");

    QString psScript = QString("Remove-Item '%1' -Force").arg(shortcutPath);

    return executePs(psScript);
}

bool TaskScheduler::isStartupShortcutExists(const QString &taskName)
{
    QString home = QDir::homePath();
    QString startupDir =
        home +
        "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup";
    QString shortcutPath = QDir(startupDir).absoluteFilePath(taskName + ".lnk");

    return QFile::exists(shortcutPath);
}

bool TaskScheduler::executePs(const QString &psScript)
{
    QProcess process;
    process.setProgram("powershell");
    process.setArguments({"-ExecutionPolicy", "Bypass", "-Command", psScript});

    process.start();
    if (!process.waitForStarted(5000))
    {
        qDebug() << "PowerShell 启动失败:" << process.errorString();
        return false;
    }

    if (process.waitForFinished(5000) && process.exitCode() == 0)
    {
        qDebug() << "PowerShell 执行成功";
        return true;
    }
    else
    {
        QString error = QString::fromLocal8Bit(process.readAllStandardError());
        qDebug() << "PowerShell 执行失败:" << error;
    }

    return false;
}
