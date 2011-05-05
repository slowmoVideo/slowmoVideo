#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include <QDialog>

namespace Ui {
    class RenderDialog;
}

class RenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderDialog(QWidget *parent = 0);
    ~RenderDialog();

private:
    Ui::RenderDialog *ui;
};

#endif // RENDERDIALOG_H
