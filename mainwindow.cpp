#include "./ui_mainwindow.h"
#include "mainwindow.h"
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QScreen>
#include <QStyle>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_settings =
        new QSettings(QCoreApplication::applicationDirPath() + "/config.ini",
                      QSettings::IniFormat);

    // 安装本地事件过滤器以处理全局快捷键
    QApplication::instance()->installNativeEventFilter(this);
    setupGlobalHotkeys();

    init();
}

MainWindow::~MainWindow()
{
    unregisterGlobalHotkeys();
    QApplication::instance()->removeNativeEventFilter(this);
    m_thread->quit();
    m_thread->wait();
    delete m_settings;
    delete m_processMonitor;
    delete m_cpu;
    delete m_mem;
    delete ui;
}

void MainWindow::refresh()
{
    ui->cpuContent->setText(QString("<span style='color:blue'>%1%</span>")
                                .arg(m_cpu->getUsage(), 5, 'f', 1, ' '));

    double memUsage = m_mem->getUsage();
    double memTotal = m_mem->getTotal();
    ui->memContent->setText(
        QString("<span style='color:blue'>(%1G / %2G) %3%</span>")
            .arg(memUsage, 0, 'f', 1)
            .arg(memTotal, 0, 'f', 1)
            .arg(memUsage / memTotal * 100, 4, 'f', 1));
}

void MainWindow::on_sliderCtrl_valueChanged(int value)
{
    double factor = speedFactor(value);

    m_processMonitor->changeSpeed(factor);
    ui->sliderCtrl->setToolTip(QString("%1倍").arg(factor, 4, 'f', 2));
    ui->sliderLabel->setText(QString("x%1倍").arg(factor, 4, 'f', 2));
    m_settings->setValue(CONFIG_SLIDERVALUE_KEY, value);
    m_settings->sync();
}

void MainWindow::on_processNameFilter_textChanged(const QString &text)
{
    m_processMonitor->setFilter(text);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    {
        switch (reason)
        {
            case QSystemTrayIcon::Trigger:  // 单击
                if (isVisible())
                    hide();
                else
                {
                    show();
                    showNormal();
                    activateWindow();
                }
                break;
            case QSystemTrayIcon::DoubleClick:  // 双击
                show();
                showNormal();
                activateWindow();
                break;
            default:
                break;
        }
    }
}

void MainWindow::createTray()
{
    // 创建系统托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icons/images/icon.ico"));
    trayIcon->setToolTip("OpenSpeedy");

    // 创建托盘菜单
    trayMenu = new QMenu(this);

    // 创建动作
    showAction = new QAction("显示", this);
    hideAction = new QAction("隐藏", this);
    quitAction = new QAction("退出", this);

    // 连接信号和槽
    connect(showAction, &QAction::triggered, this,
            [&]
            {
                showNormal();
                activateWindow();
            });
    connect(hideAction, &QAction::triggered, this, &MainWindow::hide);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    // 添加动作到菜单
    trayMenu->addAction(showAction);
    trayMenu->addAction(hideAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    // 设置托盘图标的菜单
    trayIcon->setContextMenu(trayMenu);

    // 处理托盘图标的点击事件
    connect(trayIcon, &QSystemTrayIcon::activated, this,
            &MainWindow::iconActivated);

    // 显示托盘图标
    trayIcon->show();
}

double MainWindow::speedFactor(int sliderValue)
{
    double factor = 1.0;

    if (sliderValue >= 1 && sliderValue < 5)
    {
        factor = sliderValue * 0.25 + 1;
    }
    else if (sliderValue >= 5 && sliderValue < 7)
    {
        factor = sliderValue * 0.5;
    }
    else if (sliderValue >= 7 && sliderValue < 9)
    {
        factor = 3 * (sliderValue - 7) + 4;
    }
    else if (sliderValue >= 9)
    {
        factor = 5 * (sliderValue - 9) + 10;
    }
    else if (sliderValue < 0)
    {
        factor = (double)(30 + sliderValue) / 30;
    }
    else
    {
        factor = 1.0;
    }

    return factor;
}

void MainWindow::setupGlobalHotkeys()
{
    HWND hwnd = (HWND)winId();

    // Ctrl + Alt + ↑ - 增加速度
    RegisterHotKey(hwnd, HOTKEY_INCREASE_SPEED, MOD_CONTROL | MOD_ALT, VK_UP);

    // Ctrl + Alt + ↓ - 降低速度
    RegisterHotKey(hwnd, HOTKEY_DECREASE_SPEED, MOD_CONTROL | MOD_ALT, VK_DOWN);

    // Ctrl + Alt + 0 - 重置速度
    RegisterHotKey(hwnd, HOTKEY_RESET_SPEED, MOD_CONTROL | MOD_ALT, '0');

    qDebug() << "全局快捷键已注册:";
    qDebug() << "Ctrl+Alt+↑ - 增加速度";
    qDebug() << "Ctrl+Alt+↓ - 降低速度";
    qDebug() << "Ctrl+Alt+0 - 重置速度";
}

void MainWindow::unregisterGlobalHotkeys()
{
    HWND hwnd = (HWND)winId();

    UnregisterHotKey(hwnd, HOTKEY_INCREASE_SPEED);
    UnregisterHotKey(hwnd, HOTKEY_DECREASE_SPEED);
    UnregisterHotKey(hwnd, HOTKEY_RESET_SPEED);

    qDebug() << "全局快捷键已注销";
}

void MainWindow::recreateTray()
{
    qDebug() << "重绘托盘";
    resize(960, 640);
    delete trayMenu;
    delete trayIcon;
    delete showAction;
    delete hideAction;
    delete quitAction;
    createTray();
}

void MainWindow::init()
{
    m_processMonitor = new ProcessMonitor(
        ui->processMonitorWidget, ui->processMonitorLabel, ui->injector32Status,
        ui->injector64Status, m_settings, nullptr);
    m_thread = new QThread(this);

    connect(m_thread, &QThread::started, m_processMonitor,
            &ProcessMonitor::start);
    connect(m_thread, &QThread::finished, m_processMonitor,
            &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    connect(QGuiApplication::primaryScreen(),
            &QScreen::logicalDotsPerInchChanged, this,
            &MainWindow::recreateTray);
    m_thread->start();
    m_processMonitor->start();

    createTray();
    QApplication::setQuitOnLastWindowClosed(false);

    ui->osContent->setText(winutils::getWindowsVersion());
    m_cpu = new CpuUtils();
    m_cpu->init();
    m_mem = new MemUtils();
    m_mem->init();
    m_timer = new QTimer();

    refresh();
    connect(m_timer, &QTimer::timeout, this, &MainWindow::refresh);
    m_timer->start(1000);

    int value = qBound(ui->sliderCtrl->minimum(),
                       m_settings->value(CONFIG_SLIDERVALUE_KEY, 1).toInt(),
                       ui->sliderCtrl->maximum());
    ui->sliderCtrl->setValue(value);

    connect(ui->menuAbout, &QMenu::aboutToShow,
            [this]
            {
                ui->menuAbout->hide();
                QTimer::singleShot(
                    50,
                    [this]()
                    {
                        m_aboutDlg.setModal(true);
                        m_aboutDlg.show();
                        m_aboutDlg.activateWindow();
                        m_aboutDlg.raise();
                        m_aboutDlg.setFocus(Qt::ActiveWindowFocusReason);
                    });
            });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 如果托盘图标可见，则隐藏窗口而不是关闭
    if (trayIcon->isVisible())
    {
        hide();

        // 阻止事件继续传播，防止应用关闭
        event->ignore();
    }
    else
    {
        // 如果没有托盘图标，则正常关闭
        event->accept();
    }
}

bool MainWindow::nativeEventFilter(const QByteArray &eventType,
                                   void *message,
                                   long *result)
{
    Q_UNUSED(result)

    if (eventType == "windows_generic_MSG" ||
        eventType == "windows_dispatcher_MSG")
    {
        MSG *msg = static_cast<MSG *>(message);

        if (msg->message == WM_HOTKEY)
        {
            static qint64 lastSoundTime = 0;
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            bool canPlaySound = (currentTime - lastSoundTime > 100);
            int hotkeyId = msg->wParam;

            switch (hotkeyId)
            {
                case HOTKEY_INCREASE_SPEED:
                {
                    int currentValue = ui->sliderCtrl->value();
                    if (currentValue < ui->sliderCtrl->maximum())
                    {
                        if (canPlaySound)
                        {
                            Beep(400, 5);
                            lastSoundTime = currentTime;
                        }
                        ui->sliderCtrl->setValue(currentValue + 1);
                        qDebug() << "全局快捷键: 增加速度到"
                                 << speedFactor(currentValue + 1);
                    }
                }
                break;

                case HOTKEY_DECREASE_SPEED:
                {
                    int currentValue = ui->sliderCtrl->value();
                    if (currentValue > ui->sliderCtrl->minimum())
                    {
                        if (canPlaySound)
                        {
                            Beep(400, 5);
                            lastSoundTime = currentTime;
                        }
                        ui->sliderCtrl->setValue(currentValue - 1);
                        qDebug() << "全局快捷键: 降低速度到"
                                 << speedFactor(currentValue - 1);
                    }
                }
                break;

                case HOTKEY_RESET_SPEED:
                    if (canPlaySound)
                    {
                        Beep(1600, 5);
                        lastSoundTime = currentTime;
                    }
                    ui->sliderCtrl->setValue(0);
                    qDebug() << "全局快捷键: 重置速度";
                    break;
            }

            return true;  // 事件已处理
        }
    }

    return false;  // 让其他过滤器处理
}
