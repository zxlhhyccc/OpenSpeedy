#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "SpeedPatch/speedpatch.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_processMonitor = new ProcessMonitor(ui->processMonitorWidget, ui->processMonitorLabel, nullptr);
    m_thread = new QThread(this);
    //m_processMonitor->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_processMonitor, &ProcessMonitor::start);
    connect(m_thread, &QThread::finished, m_processMonitor, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_thread->start();
    m_processMonitor->start();
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
        speedFactor = value - 3;
    }
    else if (value < 0)
    {
        speedFactor = (double)(10+value)/10;
    }
    else
    {
        speedFactor = 1;
    }
    SetSpeedFactor(speedFactor);
    ui->sliderCtrl->setToolTip(QString("%1倍").arg(speedFactor));
    ui->sliderLabel->setText(QString("x%1倍").arg(speedFactor));
}


void MainWindow::on_processNameFilter_textChanged(const QString &text)
{
    m_processMonitor->setFilter(text);
}

