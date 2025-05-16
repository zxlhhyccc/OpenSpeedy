#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <QObject>
#include <QTreeWidget>
#include <QLabel>
#include <QTimer>
#include <QThread>
#include <windows.h>
#include <tlhelp32.h>
#include <QMap>
#include "winutils.h"
class ProcessMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ProcessMonitor(QTreeWidget *treeWidget, QLabel *label, QObject *parent = nullptr);
    ~ProcessMonitor();

    // 设置刷新间隔（毫秒）
    void setInterval(int msec);
    // 设置过滤
    void setFilter(QString processName);

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

    // 图标缓存
    QHash<QString, QIcon> m_iconCache;

    // 存储进程ID到TreeWidgetItem的映射
    QMap<DWORD, QTreeWidgetItem*> m_processItemMap;
    // 存储需要加速的进程
    QSet<QString> m_speedupItems;

    void init();

    void dump();

    void update(const QList<ProcessInfo> &processList);

    QIcon getProcessIconCached(DWORD proccessId);
};

#endif // PROCESSMONITOR_H
