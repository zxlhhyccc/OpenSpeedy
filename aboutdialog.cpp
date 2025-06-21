/*
 * OpenSpeedy - Open Source Game Speed Controller
 * Copyright (C) 2025 Game1024
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QPushButton>
#include <QScreen>
AboutDialog::AboutDialog(QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确认"));
    ui->versionContent->setText(QString("%1").arg(OPENSPEEDY_VERSION));
    connect(QGuiApplication::primaryScreen(),
            &QScreen::logicalDotsPerInchChanged,
            this,
            &AboutDialog::recreate);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void
AboutDialog::recreate()
{
    layout()->invalidate();
    layout()->activate();
    adjustSize();
}
