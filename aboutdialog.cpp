#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QScreen>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionContent->setText(QString("版本：%1").arg(OPENSPEEDY_VERSION));
    connect(QGuiApplication::primaryScreen(),
            &QScreen::logicalDotsPerInchChanged, this, &AboutDialog::recreate);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint |
                   Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint |
                   Qt::CustomizeWindowHint);
    adjustSize();
}

AboutDialog::~AboutDialog() { delete ui; }

void AboutDialog::recreate() { adjustSize(); }
