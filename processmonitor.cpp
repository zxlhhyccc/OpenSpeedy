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
#include "config.h"
#include "processmonitor.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileIconProvider>
#include <QHeaderView>
#include <QList>
#include <QStyle>
#include <QtConcurrent/QtConcurrent>
#include <QtWinExtras/QtWin>
#include <psapi.h>
ProcessMonitor::ProcessMonitor(QSettings*   settings,
                               QTreeWidget* treeWidget,
                               QLabel*      treeStatusLabel,
                               QLabel*      injector32StatusLabel,
                               QLabel*      injector64StatusLabel,
                               QObject*     parent)
    : m_treeWidget(treeWidget)
    , m_treeStatusLabel(treeStatusLabel)
    , m_injector32StatusLabel(injector32StatusLabel)
    , m_injector64StatusLabel(injector64StatusLabel)
    , m_settings(settings)
{
    m_treeWidget->header()->setMinimumHeight(40);
    m_treeWidget->setColumnWidth(0, 300);
    m_treeWidget->setColumnWidth(5, 50);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->sortByColumn(0, Qt::AscendingOrder);
    m_treeWidget->resizeColumnToContents(2);
    m_treeWidget->resizeColumnToContents(3);

    connect(m_treeWidget,
            &QTreeWidget::itemChanged,
            this,
            &ProcessMonitor::onItemChanged);

    this->startBridge32();
    this->startBridge64();

    init();
    refresh();
}

ProcessMonitor::~ProcessMonitor()
{
    this->terminalBridge();
    delete m_bridge32;
    delete m_bridge64;
}

void
ProcessMonitor::setFilter(QString processName)
{
    m_filter = processName.toLower();
}

void
ProcessMonitor::refresh()
{
    healthcheckBridge();
    QList<ProcessInfo> processList = winutils::getProcessList();
    if (m_filter == "")
    {
        update(processList);
        m_treeStatusLabel->setText(QString(tr("搜索到%1个进程, 已过滤展示%2个"))
                                   .arg(processList.size())
                                   .arg(processList.size()));
    }
    else
    {
        QList<ProcessInfo> filtered;
        for (ProcessInfo info : processList)
        {
            if (info.name.toLower().contains(m_filter))
                filtered.append(info);
        }
        update(filtered);
        m_treeStatusLabel->setText(QString(tr("搜索到%1个进程, 已过滤展示%2个"))
                                   .arg(processList.size())
                                   .arg(filtered.size()));
    }
}

void
ProcessMonitor::start()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ProcessMonitor::refresh);
    m_timer->start(1000);
}

void
ProcessMonitor::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (column == 5 && (item->flags() & Qt::ItemIsUserCheckable))
    {
        bool state = m_treeWidget->blockSignals(true);

        Qt::CheckState checkState = item->checkState(column);
        QString processName = item->text(0);
        DWORD pid = item->text(1).toLong();
        bool is64Bit = item->text(3) == "x64" ? true : false;
        if (checkState == Qt::Checked)
        {
            m_targetNames.insert(processName);
            this->injectDll(pid, is64Bit);
            item->setText(5, tr("加速中"));
            for (int col = 0; col < item->columnCount(); ++col)
            {
                item->setBackground(col, QBrush(QColor("#f3e5f5")));
                item->setForeground(col, QBrush(QColor("#7b1fa2")));
            }
            qDebug() << processName << "勾选";
            dump();
        }
        else
        {
            m_targetNames.remove(processName);
            this->unhookDll(pid, is64Bit);
            item->setText(5, "");
            for (int col = 0; col < item->columnCount(); ++col)
            {
                item->setBackground(col, QBrush());
                item->setForeground(col, QBrush());
            }
            qDebug() << processName << "取消勾选";
            dump();
        }

        m_treeWidget->blockSignals(state);
    }
}

void
ProcessMonitor::init()
{
    QStringList targetNames =
        m_settings->value(CONFIG_TARGETNAMES_KEY).toStringList();
    m_targetNames = QSet<QString>(targetNames.begin(), targetNames.end());
}

void
ProcessMonitor::dump()
{
    m_settings->setValue(CONFIG_TARGETNAMES_KEY,
                         QStringList(m_targetNames.values()));
}

void
ProcessMonitor::update(const QList<ProcessInfo>& processList)
{
    // 跟踪现有进程，用于确定哪些已终止
    QSet<DWORD> currentPids;
    for (const ProcessInfo& info : processList)
    {
        currentPids.insert(info.pid);
    }

    // 移除已终止的进程
    QMutableMapIterator<DWORD, QTreeWidgetItem*> i(m_processItems);
    while (i.hasNext())
    {
        i.next();
        if (!currentPids.contains(i.key()))
        {
            QTreeWidgetItem* item = i.value();
            QTreeWidgetItem* parent = item->parent();

            if (parent)
            {
                parent->removeChild(item);
            }
            else
            {
                auto index = m_treeWidget->indexOfTopLevelItem(item);
                m_treeWidget->takeTopLevelItem(index);
            }

            while (item->childCount() > 0)
            {
                QTreeWidgetItem* child = item->takeChild(0);
                if (parent)
                {
                    parent->addChild(child);
                }
                else
                {
                    m_treeWidget->addTopLevelItem(child);
                }
            }

            // 删除节点
            delete item;

            // 从映射中移除
            i.remove();
        }
    }

    // 添加或更新进程信息
    for (const ProcessInfo& info : processList)
    {
        if (m_processItems.contains(info.pid))
        {
            // 更新已存在的进程信息
            QTreeWidgetItem* item = m_processItems[info.pid];
            item->setText(1, QString::number(info.pid));
            item->setData(1, Qt::UserRole, (long long)info.pid);
            item->setText(2,
                          QString("%1 MB").arg(info.memoryUsage / 1024 / 1024));
            item->setData(2, Qt::UserRole, (uint)info.memoryUsage);

            QString arch = info.is64Bit ? "x64" : "x86";
            item->setText(3, arch);
            QString priority;
            switch (info.priorityClass)
            {
            case HIGH_PRIORITY_CLASS:
                priority = tr("高");
                break;
            case NORMAL_PRIORITY_CLASS:
                priority = tr("中");
                break;
            case IDLE_PRIORITY_CLASS:
                priority = tr("低");
                break;
            case REALTIME_PRIORITY_CLASS:
                priority = tr("实时");
                break;
            default:
                priority = tr("未知");
                break;
            }
            item->setText(4, priority);
            if (m_targetNames.contains(info.name))
            {
                if (item->checkState(5) == Qt::Unchecked)
                {
                    item->setCheckState(5, Qt::Checked);
                }
            }
            else
            {
                if (item->checkState(5) == Qt::Checked)
                {
                    item->setCheckState(5, Qt::Unchecked);
                }
            }
        }
        else
        {
            // 添加新进程
            QTreeWidgetItem* item = new SortTreeWidgetItem();

            item->setText(0, info.name);
            item->setData(0, Qt::UserRole, info.name.toLower());
            item->setText(1, QString::number(info.pid));
            item->setData(1, Qt::UserRole, (long long)info.pid);
            item->setText(2,
                          QString("%1 MB").arg(info.memoryUsage / 1024 / 1024));
            item->setData(2, Qt::UserRole, (uint)info.memoryUsage);

            QString arch = info.is64Bit ? "x64" : "x86";
            item->setText(3, arch);

            // 加载进程图标
            item->setIcon(0, getProcessIconCached(info.pid));

            QString priority;
            switch (info.priorityClass)
            {
            case HIGH_PRIORITY_CLASS:
                priority = tr("高");
                break;
            case NORMAL_PRIORITY_CLASS:
                priority = tr("中");
                break;
            case IDLE_PRIORITY_CLASS:
                priority = tr("低");
                break;
            case REALTIME_PRIORITY_CLASS:
                priority = tr("实时");
                break;
            default:
                priority = tr("未知");
                break;
            }
            item->setText(4, priority);
            item->setCheckState(5, Qt::Unchecked);
            m_treeWidget->addTopLevelItem(item);
            m_processItems[info.pid] = item;
        }
    }
}

void
ProcessMonitor::injectDll(DWORD processId, bool is64Bit)
{
    QString cmd = QString("inject %1\n").arg(processId);
    if (!is64Bit)
    {
        m_bridge32->write(cmd.toUtf8(), cmd.size());
        m_bridge32->waitForBytesWritten();
    }
    else
    {
        m_bridge64->write(cmd.toUtf8(), cmd.size());
        m_bridge64->waitForBytesWritten();
    }
}

void
ProcessMonitor::unhookDll(DWORD processId, bool is64Bit)
{
    QString cmd = QString("unhook %1\n").arg(processId);
    if (!is64Bit)
    {
        m_bridge32->write(cmd.toUtf8(), cmd.size());
        m_bridge32->waitForBytesWritten();
    }
    else
    {
        m_bridge64->write(cmd.toUtf8(), cmd.size());
        m_bridge64->waitForBytesWritten();
    }
}

void
ProcessMonitor::startBridge32()
{
    m_bridge32 = new QProcess();
    QStringList params32;
    m_bridge32->start(BRIDGE32_EXE, params32);
    if (!m_bridge32->waitForStarted())
    {
        qDebug() << "32位桥接子进程启动失败";
    }
    else
    {
        qDebug() << "32位桥接子进程已启动";
    }
    m_bridge32->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_bridge32,
            &QProcess::readyReadStandardOutput,
            [&]()
    {
        QByteArray data = m_bridge32->readAllStandardOutput();
        qDebug() << "收到输出:" << QString(data).trimmed();
    });
}

void
ProcessMonitor::startBridge64()
{
    m_bridge64 = new QProcess();
    QStringList params64;
    m_bridge64->start(BRIDGE64_EXE, params64);
    if (!m_bridge64->waitForStarted())
    {
        qDebug() << "64位桥接子进程启动失败";
    }
    else
    {
        qDebug() << "64位桥接子进程已启动";
    }
    m_bridge64->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_bridge64,
            &QProcess::readyReadStandardOutput,
            [&]()
    {
        QByteArray data = m_bridge64->readAllStandardOutput();
        qDebug() << "收到输出:" << QString(data).trimmed();
    });
}

void
ProcessMonitor::healthcheckBridge()
{
    if (this->m_bridge32->state() == QProcess::Running)
    {
        m_injector32StatusLabel->setStyleSheet("color: green");
        m_injector32StatusLabel->setText(tr("正常"));
    }
    else
    {
        m_injector32StatusLabel->setStyleSheet("color: red");
        m_injector32StatusLabel->setText(tr("异常退出"));
    }

    if (this->m_bridge64->state() == QProcess::Running)
    {
        m_injector64StatusLabel->setStyleSheet("color: green");
        m_injector64StatusLabel->setText(tr("正常"));
    }
    else
    {
        m_injector64StatusLabel->setStyleSheet("color:red");
        m_injector64StatusLabel->setText(tr("异常退出"));
    }
}

void
ProcessMonitor::terminalBridge()
{
    QString cmd = QString("exit\n");
    m_bridge32->write(cmd.toUtf8(), cmd.size());
    m_bridge32->waitForBytesWritten();
    m_bridge64->write(cmd.toUtf8(), cmd.size());
    m_bridge64->waitForBytesWritten();
}

void
ProcessMonitor::changeSpeed(double factor)
{
    QString cmd = QString("change %1\n").arg(factor);
    m_bridge32->write(cmd.toUtf8(), cmd.size());
    m_bridge32->waitForBytesWritten();
    m_bridge64->write(cmd.toUtf8(), cmd.size());
    m_bridge64->waitForBytesWritten();
}

QIcon
ProcessMonitor::getProcessIconCached(DWORD processId)
{
    QString processPath = winutils::getProcessPath(processId);
    if (m_iconCache.contains(processPath))
    {
        return m_iconCache[processPath];
    }

    QIcon icon = getProcessIcon(processPath);
    m_iconCache.insert(processPath, icon);
    return icon;
}

QIcon
ProcessMonitor::getDefaultIcon(const QString& processName)
{
    // 根据进程名称或类型提供更有针对性的默认图标
    if (processName != "")
    {
        // 使用SHGetFileInfo获取.exe文件的图标
        SHFILEINFO sfi = {};
        QIcon icon;
        if (SHGetFileInfo(reinterpret_cast<LPCWSTR>(processName.utf16()),
                          FILE_ATTRIBUTE_NORMAL,
                          &sfi,
                          sizeof(SHFILEINFO),
                          SHGFI_USEFILEATTRIBUTES | SHGFI_ICON |
                          SHGFI_SMALLICON))
        {
            // 将HICON转换为QIcon
            icon = QtWin::fromHICON(sfi.hIcon);

            // 释放图标资源
            DestroyIcon(sfi.hIcon);
        }
        return icon;
    }
    else
    {
        // 使用SHGetFileInfo获取.exe文件的图标
        SHFILEINFO sfi = {};
        QIcon icon;
        if (SHGetFileInfo(reinterpret_cast<LPCWSTR>(L".exe"),
                          FILE_ATTRIBUTE_NORMAL,
                          &sfi,
                          sizeof(SHFILEINFO),
                          SHGFI_USEFILEATTRIBUTES | SHGFI_ICON |
                          SHGFI_SMALLICON))
        {
            // 将HICON转换为QIcon
            icon = QtWin::fromHICON(sfi.hIcon);

            // 释放图标资源
            DestroyIcon(sfi.hIcon);
        }
        return icon;
    }
}

QIcon
ProcessMonitor::getProcessIcon(QString processPath)
{
    int lastSlashPos =
        std::max(processPath.lastIndexOf('/'), processPath.lastIndexOf('\\'));
    QString processName;
    if (lastSlashPos == -1)
    {
        processName = processPath;
    }
    processName = processPath.mid(lastSlashPos + 1);

    if (processPath.isEmpty())
    {
        qDebug() << processPath << "无法获取进程完整路径";
        return getDefaultIcon(processName);
    }

    SHFILEINFO sfi = {};
    if (SHGetFileInfo(reinterpret_cast<LPCWSTR>(processPath.utf16()),
                      FILE_ATTRIBUTE_NORMAL,
                      &sfi,
                      sizeof(SHFILEINFO),
                      SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES))
    {
        if (sfi.hIcon)
        {
            QIcon icon = QtWin::fromHICON(sfi.hIcon);
            DestroyIcon(sfi.hIcon);
            if (!icon.isNull())
            {
                qDebug() << processPath << "通过SHGetFileInfo获取图标成功";
                return icon;
            }
        }
    }

    HICON hIcon =
        ExtractIconW(nullptr, reinterpret_cast<LPCWSTR>(processPath.utf16()), 0);
    if (hIcon)
    {
        QIcon icon = QtWin::fromHICON(hIcon);
        DestroyIcon(hIcon);
        if (!icon.isNull())
            return icon;
    }

    QFileInfo fileInfo(processPath);
    if (fileInfo.exists())
    {
        // 使用Qt的QFileIconProvider获取文件图标
        QFileIconProvider iconProvider;
        return iconProvider.icon(fileInfo);
    }
    qDebug() << processPath << "无法获取进程图标使用默认图标";

    return getDefaultIcon(processName);
}
