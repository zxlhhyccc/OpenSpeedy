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
#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <windows.h>
#include "config.h"
#include "qsinglekeysequenceedit.h"
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QMap>
#include <QSettings>
#include <QSpinBox>

namespace Ui
{
class PreferenceDialog;
}

#define CONFIG_HOTKEY_SPEEDUP "Hotkey/SpeedUp"
#define CONFIG_HOTKEY_SPEEDDOWN "Hotkey/SpeedDown"
#define CONFIG_HOTKEY_RESETSPEED "Hotkey/ResetSpeed"
#define CONFIG_HOTKEY_SHIFT1 "Hotkey/Shift1"
#define CONFIG_HOTKEY_SHIFT2 "Hotkey/Shift2"
#define CONFIG_HOTKEY_SHIFT3 "Hotkey/Shift3"
#define CONFIG_HOTKEY_SHIFT4 "Hotkey/Shift4"
#define CONFIG_HOTKEY_SHIFT5 "Hotkey/Shift5"

#define CONFIG_SHIFT1_VALUE "Shift/Shift1Value"
#define CONFIG_SHIFT2_VALUE "Shift/Shift2Value"
#define CONFIG_SHIFT3_VALUE "Shift/Shift3Value"
#define CONFIG_SHIFT4_VALUE "Shift/Shift4Value"
#define CONFIG_SHIFT5_VALUE "Shift/Shift5Value"
#define CONFIG_INCREASE_STEP "Shift/IncreaseStep"
#define CONFIG_DECREASE_STEP "Shift/DecreaseStep"

#define DEFAULT_SHIFT1_VALUE 10.0
#define DEFAULT_SHIFT2_VALUE 20.0
#define DEFAULT_SHIFT3_VALUE 30.0
#define DEFAULT_SHIFT4_VALUE 40.0
#define DEFAULT_SHIFT5_VALUE 50.0
#define DEFAULT_INCREASE_STEP 1
#define DEFAULT_DECREASE_STEP 1

#define DEFAULT_HOTKEY_SPEEDUP "Ctrl+Alt+Up"
#define DEFAULT_HOTKEY_SPEEDDOWN "Ctrl+Alt+Down"
#define DEFAULT_HOTKEY_RERSETSPEED "Ctrl+Alt+0"
#define DEFAULT_HOTKEY_SHIFT1 "Ctrl+Alt+1"
#define DEFAULT_HOTKEY_SHIFT2 "Ctrl+Alt+2"
#define DEFAULT_HOTKEY_SHIFT3 "Ctrl+Alt+3"
#define DEFAULT_HOTKEY_SHIFT4 "Ctrl+Alt+4"
#define DEFAULT_HOTKEY_SHIFT5 "Ctrl+Alt+5"

class PreferenceDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PreferenceDialog(HWND hMainWindow,
                              QSettings* settings,
                              QLabel* increaseSpeedLabel,
                              QLabel* decreaseSpeedLabel,
                              QLabel* resetSpeedLabel,
                              QWidget* parent = nullptr);
    ~PreferenceDialog();

    int getIncreaseStep();

    int getDecreaseStep();

    double getShift1();

    double getShift2();

    double getShift3();

    double getShift4();

    double getShift5();

  public slots:
    void show();

  private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void recreate();

  private:
    void setupGlobalHotkeys();

    void unregisterGlobalHotkeys();

    void loadShortcut(int id,
                      const QString& config,
                      const QString& defaultValue);

    void dumpShortcut(const QString& config, const QString& keyText);
    void dump();

    void updateShortcut(int id, QSingleKeySequenceEdit* keyEdit);
    void update();

    void redrawSpinBox(QSpinBox* spinbox, int value);
    void redrawSpinBox(QDoubleSpinBox* spinbox, double value);
    void redrawKeyEdit(QSingleKeySequenceEdit* keyEdit, int id);
    void redraw();

    Ui::PreferenceDialog* ui;

    QSettings* m_settings;

    QLabel* m_increaseSpeedLabel;
    QLabel* m_decreaseSpeedLabel;
    QLabel* m_resetSpeedLabel;

    QMap<int, QtCombinedKey> m_shortcuts;

    double m_shift1Value;
    double m_shift2Value;
    double m_shift3Value;
    double m_shift4Value;
    double m_shift5Value;

    int m_increaseStep;
    int m_decreaseStep;

    HWND m_mainwindow;
};

#endif // PREFERENCEDIALOG_H
