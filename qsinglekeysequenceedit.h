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
