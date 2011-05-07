#include "progressDialogExtractFrames.h"
#include "ui_progressDialogExtractFrames.h"

#include <QDebug>

ProgressDialogExtractFrames::ProgressDialogExtractFrames(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialogExtractFrames)
{
    ui->setupUi(this);
    ui->progOrig->setValue(0);
    ui->progThumbs->setValue(0);

    ui->bOk->setEnabled(false);
    bool b = true;
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    Q_ASSERT(b);
}

ProgressDialogExtractFrames::~ProgressDialogExtractFrames()
{
    delete ui;
}

void ProgressDialogExtractFrames::slotExtractionFinished(FrameSize frameSize)
{
    qDebug() << "Frame size completed: " << frameSize;
    switch (frameSize) {
    case FrameSize_Orig:
        ui->progOrig->setValue(100);
        break;
    case FrameSize_Small:
        ui->progThumbs->setValue(100);
        break;
    }
    if (ui->progOrig->value() == 100 && ui->progThumbs->value() == 100) {
        ui->bOk->setEnabled(true);
    }
}

void ProgressDialogExtractFrames::slotProgressUpdated(FrameSize frameSize, int value)
{
    switch(frameSize) {
    case FrameSize_Orig:
        ui->progOrig->setValue(value);
        break;
    case FrameSize_Small:
        ui->progThumbs->setValue(value);
        break;
    }
}
