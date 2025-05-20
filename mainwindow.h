#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "ProcessMonitor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
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
    void on_sliderCtrl_valueChanged(int value);

    void on_processNameFilter_textChanged(const QString &arg1);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
private:
    Ui::MainWindow *ui;
    QThread *m_thread;
    ProcessMonitor *m_processMonitor;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showAction;
    QAction *hideAction;
    QAction *quitAction;

    void createTray();

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
