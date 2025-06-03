#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "cpuutils.h"
#include "memutils.h"
#include "processmonitor.h"
#include <QAbstractNativeEventFilter>
#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>

#define CONFIG_SLIDERVALUE_KEY "MainWindow/SliderValue"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
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

    QSettings *m_settings;

    CpuUtils *m_cpu;
    MemUtils *m_mem;

    void init();

    void createTray();

    double speedFactor(int sliderValue);

    // 热键ID定义
    enum HotkeyIds
    {
        HOTKEY_INCREASE_SPEED = 1001,
        HOTKEY_DECREASE_SPEED = 1002,
        HOTKEY_RESET_SPEED = 1003,
    };

    void setupGlobalHotkeys();

    void unregisterGlobalHotkeys();

   protected:
    void closeEvent(QCloseEvent *event) override;

    bool nativeEventFilter(const QByteArray &eventType,
                           void *message,
                           long *result) override;
};
#endif  // MAINWINDOW_H
