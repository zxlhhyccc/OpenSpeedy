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
#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <windows.h>
#include <QObject>
#include <QString>
class TaskScheduler : public QObject
{
    Q_OBJECT
  public:
    explicit TaskScheduler(QObject *parent = nullptr);
    ~TaskScheduler();

    bool createStartupTask(const QString &taskName,
                           const QString &executablePath);
    bool deleteTask(const QString &taskName);
    bool enableTask(const QString &taskName, bool enable);
    bool isTaskExists(const QString &taskName);
    bool execute(const QStringList &arguments);

    bool createStartupShortcut(const QString &taskName,
                               const QString &executablePath);
    bool deleteStartupShortcut(const QString &taskName);
    bool isStartupShortcutExists(const QString &taskName);
    bool executePs(const QString &psScript);
};

#endif // TASKSCHEDULER_H
