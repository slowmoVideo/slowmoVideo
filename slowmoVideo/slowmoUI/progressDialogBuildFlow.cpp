#include "progressDialogBuildFlow.h"
#include "ui_progressDialogBuildFlow.h"

#include <QDebug>

ProgressDialogBuildFlow::ProgressDialogBuildFlow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialogBuildFlow)
{
    ui->setupUi(this);
    ui->currentFile->setText(QString());

    m_progressRange = 100;
    ui->progressBar->setRange(0, m_progressRange);
}

ProgressDialogBuildFlow::~ProgressDialogBuildFlow()
{
    delete ui;
}


void ProgressDialogBuildFlow::setProgressRange(int max)
{
    m_progressRange = max;
    ui->progressBar->setRange(0, m_progressRange);
}

void ProgressDialogBuildFlow::slotFlowFinished()
{
    emit accepted();
}

void ProgressDialogBuildFlow::slotProgressUpdated(int value)
{
    Q_ASSERT(value <= 100);
    ui->progressBar->setValue(value);
    qDebug() << "PDBF: New value: " << value;
}

void ProgressDialogBuildFlow::slotCurrentFile(const QString &name)
{
    ui->currentFile->setText(name);
    qDebug() << "PDBF: Current file: " << name;
}
