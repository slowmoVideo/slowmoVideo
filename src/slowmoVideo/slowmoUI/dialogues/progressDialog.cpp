#include "progressDialog.h"
#include "ui_progressDialog.h"

#include <QMessageBox>
#include <QtCore/QDebug>

#include "notificator.h"

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    // alas this make window transparent !
    //setWindowFlags(Qt::CustomizeWindowHint |Qt::WindowStaysOnTopHint);

    connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(slotAbortPressed()));
    connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));

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
    if (windowTitle().startsWith(tr("(Finished) "))) {
        setWindowTitle(windowTitle().remove(0, tr("(Finished) ").length()));
    }
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
        QMessageBox box(QMessageBox::Warning, tr("Aborted"), message, QMessageBox::Ok);
        box.show();
    }
    reject();
}

void ProgressDialog::slotAllTasksFinished(const QString& timePassed)
{
    ui->progress->setValue(ui->progress->maximum());
    setWorking(false);
    QString notifmsg = tr("Task finished in %1.").arg(timePassed);
    if (timePassed.length() > 0) {
        slotTaskItemDescription(notifmsg);
    } else {
        slotTaskItemDescription(tr("Task finished."));
    }
    setWindowTitle(tr("(Finished) %1").arg(windowTitle()));
// display OS notification
    Notificator* notif;
    notif = new Notificator("simple");

	
    notif->notify(Notificator::Information, windowTitle(), notifmsg);
}
