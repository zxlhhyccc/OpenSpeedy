#include "processmonitor.h"
#include <QDebug>
#include <psapi.h>
#include <QDir>
#include <QHeaderView>
#include <QList>
#include <QStyle>
#include <QFileIconProvider>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include "config.h"
ProcessMonitor::ProcessMonitor(QTreeWidget *treeWidget, QLabel *label, QObject *parent)
    : m_treeWidget(treeWidget), m_treeStatusLabel(label)
{
    m_treeWidget->setColumnWidth(0, 250);
    m_treeWidget->setColumnWidth(5, 50);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->sortByColumn(0, Qt::AscendingOrder);
    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &ProcessMonitor::onItemChanged);

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

    // 注释： 程序退出时, unhook已经注入的进程
    /*
    QList<ProcessInfo> processList = winutils::getProcessList();
    for (const auto& info: processList)
    {
        if (m_speedupItems.contains(info.name))
        {
            std::wstring dllPath = QDir::toNativeSeparators(m_dllPath).toStdWString();
            winutils::unhookDll(info.pid, dllPath);
        }
    }
    */
}

void ProcessMonitor::setFilter(QString processName)
{
    m_filter = processName.toLower();
}

void ProcessMonitor::refresh()
{
    qDebug() << "定时器触发，当前线程ID:" << QThread::currentThreadId();
    QList<ProcessInfo> processList = winutils::getProcessList();
    if (m_filter == "")
    {
        update(processList);
        m_treeStatusLabel->setText(
            QString("搜索到%1个进程, 已过滤展示%2个")
                .arg(processList.size())
                .arg(processList.size())
            );
    }
    else
    {
        QList<ProcessInfo> filtered;
        for (ProcessInfo info: processList)
        {
            if(info.name.toLower().contains(m_filter)) filtered.append(info);
        }
        update(filtered);
        m_treeStatusLabel->setText(
            QString("搜索到%1个进程, 已过滤展示%2个")
                .arg(processList.size())
                .arg(filtered.size())
            );
    }
}

void ProcessMonitor::start()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ProcessMonitor::refresh);
    m_timer->start(1000);
}

void ProcessMonitor::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == 5 && (item->flags() & Qt::ItemIsUserCheckable))
    {
        Qt::CheckState checkState = item->checkState(column);
        if (checkState == Qt::Checked)
        {
            item->setCheckState(5, Qt::Checked);
            m_speedupItems.insert(item->text(0));
            dump();
        }
        else
        {
            qDebug() << item->text(0) << "取消勾选";
            item->setCheckState(5, Qt::Unchecked);
            m_speedupItems.remove(item->text(0));
            // 启动线程任务
            DWORD processId = item->text(1).toLong();
            bool is64Bit = item->text(3) == "x64" ? true : false;
            QFuture<void> future = QtConcurrent::run([&]() {
                this->unhookDll(processId, is64Bit);
            });
            dump();
        }
    }
}

void ProcessMonitor::init()
{
    QString txtPath = QCoreApplication::applicationDirPath() + "/target.txt";
    QFile file(txtPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // 创建文本流
        QTextStream in(&file);

        // 逐行读取文件内容
        while (!in.atEnd())
        {
            // 读取一行（一个进程名称）
            QString processName = in.readLine().trimmed();

            if (!processName.isEmpty())
            {
                m_speedupItems.insert(processName);
                qDebug() << "读取到进程：" << processName;
            }
        }
        // 关闭文件
        file.close();
    }
    else
    {
        // 打开文件失败
        qDebug() << "无法打开文件：" << txtPath;
        qDebug() << "错误：" << file.errorString();
    }
}

void ProcessMonitor::dump()
{
    QString txtPath = QCoreApplication::applicationDirPath() + "/target.txt";

    // 创建文件对象
    QFile file(txtPath);

    // 以写入模式打开文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "无法打开文件进行写入:" << file.errorString();
        return;
    }

    // 创建文本流
    QTextStream out(&file);

    // 逐行写入每个字符串项
    for (const QString &item : m_speedupItems)
    {
        out << item << "\n";
    }

    // 关闭文件
    file.close();

}


void ProcessMonitor::update(const QList<ProcessInfo> &processList)
{
    // 保存当前展开状态
    QMap<DWORD, bool> expandStates;
    for (auto it = m_processItemMap.constBegin(); it != m_processItemMap.constEnd(); ++it)
    {
        expandStates[it.key()] = it.value()->isExpanded();
    }

    // 跟踪现有进程，用于确定哪些已终止
    QSet<DWORD> currentPids;
    for (const ProcessInfo &info : processList)
    {
        currentPids.insert(info.pid);
    }

    // 移除已终止的进程
    QMutableMapIterator<DWORD, QTreeWidgetItem*> i(m_processItemMap);
    while (i.hasNext()) {
        i.next();
        if (!currentPids.contains(i.key()))
        {
            QTreeWidgetItem *item = i.value();
            QTreeWidgetItem *parent = item->parent();

            if (parent)
            {
                parent->removeChild(item);
            }
            else
            {
                auto index = m_treeWidget->indexOfTopLevelItem(item);
                m_treeWidget->takeTopLevelItem(index);
            }

            while(item->childCount()>0)
            {
                QTreeWidgetItem *child = item->takeChild(0);
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
    for (const ProcessInfo &info : processList) {
        if (m_processItemMap.contains(info.pid)) {
            // 更新已存在的进程信息
            QTreeWidgetItem *item = m_processItemMap[info.pid];
            item->setText(1, QString::number(info.pid));
            item->setText(2, QString("%1 MB").arg(info.memoryUsage / 1024 / 1024));
            QString arch = info.is64Bit ? "x64" : "x86";
            item->setText(3, arch);

            QString priority;
            switch (info.priorityClass) {
            case HIGH_PRIORITY_CLASS: priority = "高"; break;
            case NORMAL_PRIORITY_CLASS: priority = "中"; break;
            case IDLE_PRIORITY_CLASS: priority = "低"; break;
            case REALTIME_PRIORITY_CLASS: priority = "实时"; break;
            default: priority = "未知"; break;
            }
            item->setText(4, priority);
            if (m_speedupItems.contains(info.name)) {
                item->setCheckState(5, Qt::Checked);
                std::wstring dllPath = QDir::toNativeSeparators(m_dllPath).toStdWString();
                QFuture<void> future = QtConcurrent::run([&]() {
                    this->injectDll(info.pid, info.is64Bit);
                });
            } else {
                item->setCheckState(5, Qt::Unchecked);
            }
        } else {
            // 添加新进程
            QTreeWidgetItem *item = new QTreeWidgetItem();

            item->setText(0, info.name);
            item->setText(1, QString::number(info.pid));
            item->setText(2, QString("%1 MB").arg(info.memoryUsage / 1024 / 1024));
            QString arch = info.is64Bit ? "x64" : "x86";
            item->setText(3, arch);

            // 加载进程图标
            item->setIcon(0, getProcessIconCached(info.pid));

            QString priority;
            switch (info.priorityClass) {
            case HIGH_PRIORITY_CLASS: priority = "高"; break;
            case NORMAL_PRIORITY_CLASS: priority = "中"; break;
            case IDLE_PRIORITY_CLASS: priority = "低"; break;
            case REALTIME_PRIORITY_CLASS: priority = "实时"; break;
            default: priority = "未知"; break;
            }
            item->setText(4, priority);
            m_speedupItems.contains(info.name) ? item->setCheckState(5, Qt::Checked) : item->setCheckState(5, Qt::Unchecked);
            m_treeWidget->addTopLevelItem(item);
            m_processItemMap[info.pid] = item;
        }
    }


    // 恢复展开状态
    for (auto it = m_processItemMap.constBegin(); it != m_processItemMap.constEnd(); ++it) {
        if (expandStates.contains(it.key())) {
            it.value()->setExpanded(expandStates[it.key()]);
        }
    }
}

void ProcessMonitor::injectDll(DWORD processId, bool is64Bit)
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

void ProcessMonitor::unhookDll(DWORD processId, bool is64Bit)
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

void ProcessMonitor::startBridge32()
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
    connect(m_bridge32, &QProcess::readyReadStandardOutput, [&]() {
        QByteArray data = m_bridge32->readAllStandardOutput();
        qDebug() << "收到输出:" << QString(data).trimmed();
    });
}

void ProcessMonitor::startBridge64()
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
    connect(m_bridge64, &QProcess::readyReadStandardOutput, [&](){
        QByteArray data = m_bridge64->readAllStandardOutput();
        qDebug() << "收到输出:" << QString(data).trimmed();
    });
}

void ProcessMonitor::terminalBridge()
{
    QString cmd = QString("exit\n");
    m_bridge32->write(cmd.toUtf8(), cmd.size());
    m_bridge32->waitForBytesWritten();
    m_bridge64->write(cmd.toUtf8(), cmd.size());
    m_bridge64->waitForBytesWritten();
}

void ProcessMonitor::changeSpeed(double factor)
{
    QString cmd = QString("change %1\n").arg(factor);
    m_bridge32->write(cmd.toUtf8(), cmd.size());
    m_bridge32->waitForBytesWritten();
    m_bridge64->write(cmd.toUtf8(), cmd.size());
    m_bridge64->waitForBytesWritten();
}

QIcon ProcessMonitor::getProcessIconCached(DWORD processId)
{
    QString processPath = winutils::getProcessPath(processId);
    if (m_iconCache.contains(processPath)) {
        return m_iconCache[processPath];
    }

    QIcon icon = getProcessIcon(processPath);
    m_iconCache.insert(processPath, icon);
    return icon;
}

QIcon ProcessMonitor::getDefaultIcon(const QString &processName)
{
    // 根据进程名称或类型提供更有针对性的默认图标
    if (processName.endsWith(".dll", Qt::CaseInsensitive)) {
        return QApplication::style()->standardIcon(QStyle::SP_DriveCDIcon);
    }
    else if (processName.contains("service", Qt::CaseInsensitive)) {
        return QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon);
    }
    else if (processName.startsWith("sys", Qt::CaseInsensitive)) {
        return QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon);
    }

    // 通用默认图标
    return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}

QIcon ProcessMonitor::getProcessIcon(QString processPath)
{
    int lastSlashPos = std::max(processPath.lastIndexOf('/'), processPath.lastIndexOf('\\'));
    QString processName;
    if (lastSlashPos == -1) {
        processName = processPath;
    }
    processName = processPath.mid(lastSlashPos + 1);

    if (processPath.isEmpty()) {
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon );  // 默认图标
    }

    QFileInfo fileInfo(processPath);
    if (fileInfo.exists()) {
        // 使用Qt的QFileIconProvider获取文件图标
        QFileIconProvider iconProvider;
        return iconProvider.icon(fileInfo);
    }

    return getDefaultIcon(processName);
}

