#ifndef SHUTTERFUNCTIONDIALOG_H
#define SHUTTERFUNCTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class ShutterFunctionDialog;
}

class ShutterFunctionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShutterFunctionDialog(QWidget *parent = 0);
    ~ShutterFunctionDialog();

private:
    Ui::ShutterFunctionDialog *ui;
};

#endif // SHUTTERFUNCTIONDIALOG_H
