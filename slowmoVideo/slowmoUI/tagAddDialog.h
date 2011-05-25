#ifndef TAGADDDIALOG_H
#define TAGADDDIALOG_H

#include <QDialog>

namespace Ui {
    class TagAddDialog;
}

class TagAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagAddDialog(QWidget *parent = 0);
    ~TagAddDialog();
    QString m_text;

private:
    Ui::TagAddDialog *ui;

private slots:
    void slotTextChanged(const QString& text);
};

#endif // TAGADDDIALOG_H
