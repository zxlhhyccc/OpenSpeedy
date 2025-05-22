#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <windows.h>
#include "winutils.h"
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QThread>
#include <QTimer>
#include <QTreeWidget>
#include <tlhelp32.h>
class ProcessMonitor : public QObject
{
    Q_OBJECT
   public:
    explicit ProcessMonitor(QTreeWidget *treeWidget,
                            QLabel *label,
                            QObject *parent = nullptr);
    ~ProcessMonitor();

    // 设置刷新间隔（毫秒）
    void setInterval(int msec);
    // 设置过滤
    void setFilter(QString processName);

    void changeSpeed(double factor);

   public slots:
    // 定时刷新槽函数
    void refresh();

    void start();

   private slots:
    void onItemChanged(QTreeWidgetItem *item, int column);

   private:
    QTreeWidget *m_treeWidget;
    QLabel *m_treeStatusLabel;
    QString m_filter;
    QTimer *m_timer = nullptr;
    QString m_dllPath;

    QProcess *m_bridge32;
    QProcess *m_bridge64;

    // 图标缓存
    QHash<QString, QIcon> m_iconCache;

    // 存储进程ID到TreeWidgetItem的映射
    QMap<DWORD, QTreeWidgetItem *> m_processItemMap;
    // 存储需要加速的进程
    QSet<QString> m_speedupItems;

    void init();

    void dump();

    void update(const QList<ProcessInfo> &processList);

    void injectDll(DWORD processId, bool is64Bit);

    void unhookDll(DWORD processId, bool is64Bit);

    void startBridge32();

    void startBridge64();

    void terminalBridge();

    // 获取进程图标
    static QIcon getProcessIcon(QString processPath);

    static QIcon getDefaultIcon(const QString &processName);

    QIcon getProcessIconCached(DWORD proccessId);
};

#endif  // PROCESSMONITOR_H
