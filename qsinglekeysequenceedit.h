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
#ifndef QSINGLEKEYSEQUENCEEDIT_H
#define QSINGLEKEYSEQUENCEEDIT_H

#include <windows.h>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QObject>
#define QKEYSEQUENCEEDIT_DEFAULT_PLACEHOLDER "点击设置热键"

struct QtCombinedKey
{
    int key;
    Qt::KeyboardModifiers modifier;
};

class QSingleKeySequenceEdit : public QKeySequenceEdit
{
    Q_OBJECT

  public:
    explicit QSingleKeySequenceEdit(QWidget* parent = nullptr);

    static UINT toVK(int qtKey, Qt::KeyboardModifiers qtMod);

    static UINT toModifier(Qt::KeyboardModifiers qtMod);

    static QtCombinedKey toQtCombinedKey(const QString& keyText);

    static QString wrapText(QString keyText);

    UINT getVK();

    UINT getModifier();

    QtCombinedKey getQtCombinedKey();

    QString getKeyText();

  protected:
    void keyPressEvent(QKeyEvent* event) override;

  private slots:
    void update();

  private:
    bool isModifierKey(int key) const;

    int m_key;
    Qt::KeyboardModifiers m_modifier;
};

#endif // QSINGLEKEYSEQUENCEEDIT_H
