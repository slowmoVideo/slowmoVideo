#ifndef TAGADDDIALOG_H
#define TAGADDDIALOG_H

#include "../project/tag_sV.h"
#include "../lib/defs_sV.hpp"
#include <QDialog>

namespace Ui {
    class TagAddDialog;
}

class TagAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagAddDialog(TagAxis defaultAxis, QWidget *parent = 0);
    ~TagAddDialog();
    QString m_text;

    Tag_sV buildTag(QPointF time);

protected:
    void keyPressEvent(QKeyEvent *);

private:
    Ui::TagAddDialog *ui;
    TagAxis m_axis;

private slots:
    void slotTextChanged(const QString& text);
    void slotUpdateAxis();
};

#endif // TAGADDDIALOG_H
