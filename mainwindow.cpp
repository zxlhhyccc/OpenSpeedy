#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCloseEvent>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_processMonitor = new ProcessMonitor(ui->processMonitorWidget, ui->processMonitorLabel, nullptr);
    m_thread = new QThread(this);

    connect(m_thread, &QThread::started, m_processMonitor, &ProcessMonitor::start);
    connect(m_thread, &QThread::finished, m_processMonitor, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
    m_processMonitor->start();

    createTray();

    QApplication::setQuitOnLastWindowClosed(false);
}

MainWindow::~MainWindow()
{
    m_thread->quit();
    m_thread->wait();
    delete m_processMonitor;
    delete ui;
}

void MainWindow::on_sliderCtrl_valueChanged(int value)
{
    double speedFactor = 0.0;

    if (value >= 1 && value < 5)
    {
        speedFactor = value*0.25+1;
    }
    else if (value >= 5 && value < 7)
    {
        speedFactor = value * 0.5;
    }
    else if (value >=7)
    {
        speedFactor = 3*(value - 7)+4;
    }
    else if (value < 0)
    {
        speedFactor = (double)(10+value)/10;
    }
    else
    {
        speedFactor = 1;
    }

    m_processMonitor->changeSpeed(speedFactor);
    ui->sliderCtrl->setToolTip(QString("%1倍").arg(speedFactor));
    ui->sliderLabel->setText(QString("x%1倍").arg(speedFactor));
}


void MainWindow::on_processNameFilter_textChanged(const QString &text)
{
    m_processMonitor->setFilter(text);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    {
        switch (reason) {
        case QSystemTrayIcon::Trigger: // 单击
            if (isVisible())
                hide();
            else {
                show();
                showNormal();
                activateWindow();
            }
            break;
        case QSystemTrayIcon::DoubleClick: // 双击
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
    connect(showAction, &QAction::triggered, this, [&]{
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
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

    // 显示托盘图标
    trayIcon->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 如果托盘图标可见，则隐藏窗口而不是关闭
    if (trayIcon->isVisible()) {
        hide();

        trayIcon->showMessage(
            "已最小化到托盘",
            "OpenSpeedy仍在后台运行。点击托盘图标可以重新打开窗口。",
            QSystemTrayIcon::Information,
            3000
            );

        // 阻止事件继续传播，防止应用关闭
        event->ignore();
    } else {
        // 如果没有托盘图标，则正常关闭
        event->accept();
    }
}

