#include "renderDialog.h"
#include "ui_renderDialog.h"

#include <QDebug>

RenderDialog::RenderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenderDialog)
{
    ui->setupUi(this);

    ui->comboFps->insertItems(0, QStringList() << "30" << "29.97" << "25" << "24");

    bool b = true;
    b &= connect(ui->bStart, SIGNAL(clicked()), this, SLOT(slotStartClicked()));
    b &= connect(ui->bStop, SIGNAL(clicked()), this, SLOT(slotStopClicked()));
    b &= connect(ui->comboFps, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFpsChanged()));
    b &= connect(ui->comboFps, SIGNAL(editTextChanged(QString)), this, SLOT(slotFpsChanged()));
    Q_ASSERT(b);

}

RenderDialog::~RenderDialog()
{
    delete ui;
}


void RenderDialog::slotRenderingAborted()
{
    ui->bStop->setEnabled(false);
    ui->bStart->setEnabled(true);
}
void RenderDialog::slotRenderingFinished()
{
    accept();
}
void RenderDialog::slotFrameRendered(qreal time, int frameNumber)
{
    ui->lblFrame->setText(QString("%1 (%2 s)").arg(frameNumber).arg(time));
}


void RenderDialog::slotStartClicked()
{
    ui->bStart->setEnabled(false);
    ui->bStop->setEnabled(true);
    emit signalContinueRendering();
}
void RenderDialog::slotStopClicked()
{
    ui->bStop->setEnabled(false);
    ui->bStart->setEnabled(true);
    emit signalAbortRendering();
}
void RenderDialog::slotFpsChanged()
{
    qDebug() << "Signal: fps changed to " << ui->comboFps->currentText();
    emit signalChangeFps(ui->comboFps->currentText().toFloat());
}

