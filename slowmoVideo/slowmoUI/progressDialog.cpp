#include "progressDialog.h"
#include "ui_progressDialog.h"

#include <QMessageBox>
#include <QtCore/QDebug>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    bool b = true;
    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(slotAbortPressed()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    Q_ASSERT(b);

    ui->bOk->setVisible(false);
    ui->bOk->setEnabled(false);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::setWorking(bool working)
{
    ui->bOk->setVisible(!working);
    ui->bOk->setEnabled(!working);
    ui->bAbort->setVisible(working);
    ui->bAbort->setEnabled(working);
}

void ProgressDialog::slotNextTask(const QString taskDescription, int taskSize)
{
    ui->lblTaskDesc->setText(taskDescription);
    ui->progress->setMaximum(taskSize);
    ui->progress->setValue(0);
    setWorking(true);
}
void ProgressDialog::slotTaskProgress(int progress)
{
    ui->progress->setValue(progress);
}
void ProgressDialog::slotTaskItemDescription(const QString desc)
{
    ui->lblTaskItemDesc->setText(desc);
    repaint();
}
void ProgressDialog::slotAbortPressed()
{
    emit signalAbortTask();
}
void ProgressDialog::slotAborted(const QString &message)
{
    if (message.length() > 0) {
        // Show message
        QMessageBox box(QMessageBox::Warning, "Aborted", message, QMessageBox::Ok);
        box.show();
    }
    reject();
}

void ProgressDialog::slotAllTasksFinished()
{
    ui->progress->setValue(ui->progress->maximum());
    setWorking(false);
}
