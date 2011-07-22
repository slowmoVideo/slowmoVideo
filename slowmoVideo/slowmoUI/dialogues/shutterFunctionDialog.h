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
    explicit ShutterFunctionDialog(float dy, float t0, QWidget *parent = 0);
    ~ShutterFunctionDialog();

private:
    Ui::ShutterFunctionDialog *ui;

    float m_dy;
    float m_t0;

private slots:
    void slotUpateCurve();
};

#endif // SHUTTERFUNCTIONDIALOG_H
