#include "progressDialog.h"
#include "ui_progressDialog.h"

#include <QMessageBox>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    bool b = true;
    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(slotAbortPressed()));
    Q_ASSERT(b);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::slotNextTask(const QString taskDescription, int taskSize)
{
    ui->lblTaskDesc->setText(taskDescription);
    ui->progress->setMaximum(taskSize);
    ui->progress->setValue(0);
}
void ProgressDialog::slotTaskProgress(int progress)
{
    ui->progress->setValue(progress);
}
void ProgressDialog::slotTaskItemDescription(const QString desc)
{
    ui->lblTaskItemDesc->setText(desc);
}
void ProgressDialog::slotAbortPressed()
{
    emit signalAbortTask();
}
void ProgressDialog::slotAborted(const QString &message)
{
    // \todo abort
    if (message.length() > 0) {
        // Show message
        QMessageBox box(QMessageBox::Warning, "Aborted", message, QMessageBox::Ok);
        box.show();
    }
    reject();
}

void ProgressDialog::slotAllTasksFinished()
{
    accept();
}
