#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cpuutils.h"
#include "memutils.h"
#include "processmonitor.h"
#include <QMainWindow>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   private slots:
    void refresh();

    void on_sliderCtrl_valueChanged(int value);

    void on_processNameFilter_textChanged(const QString &arg1);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void recreateTray();

   private:
    Ui::MainWindow *ui;
    QThread *m_thread;
    ProcessMonitor *m_processMonitor;
    QTimer *m_timer;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showAction;
    QAction *hideAction;
    QAction *quitAction;

    CpuUtils *m_cpu;
    MemUtils *m_mem;

    void createTray();

   protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif  // MAINWINDOW_H
