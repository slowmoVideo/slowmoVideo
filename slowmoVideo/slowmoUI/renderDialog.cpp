#include "renderDialog.h"
#include "ui_renderDialog.h"

#include "../project/project_sV.h"

#include <QDebug>

RenderDialog::RenderDialog(const Project_sV *project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenderDialog),
    m_project(project)
{
    ui->setupUi(this);

    ui->comboFps->insertItems(0, QStringList() << "30" << "29.97" << "25" << "24");
    for (int i = 0; i < ui->comboFps->count(); i++) {
        // TODO
//        if (qAbs(ui->comboFps->itemText(i).toFloat() - project->fpsOut()) < .01) {
//            ui->comboFps->setCurrentIndex(i);
//            break;
//        }
    }

    ui->comboOutputSize->insertItem(0, enumStr(FrameSize_Orig), QVariant(FrameSize_Orig));
    ui->comboOutputSize->insertItem(0, enumStr(FrameSize_Small), QVariant(FrameSize_Small));


    bool b = true;
    b &= connect(ui->bStart, SIGNAL(clicked()), this, SLOT(slotStartClicked()));
    b &= connect(ui->bStop, SIGNAL(clicked()), this, SLOT(slotStopClicked()));
    b &= connect(ui->comboFps, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFpsChanged()));
    b &= connect(ui->comboFps, SIGNAL(editTextChanged(QString)), this, SLOT(slotFpsChanged()));
    b &= connect(ui->comboOutputSize, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRenderFrameSizeChanged()));
    Q_ASSERT(b);

    slotFpsChanged();
    slotRenderFrameSizeChanged();
}

RenderDialog::~RenderDialog()
{
    delete ui;
}


void RenderDialog::slotRenderingAborted()
{
    ui->bStop->setEnabled(false);
    ui->bStart->setEnabled(true);
    ui->comboFps->setEnabled(true);
    ui->comboOutputSize->setEnabled(true);
}
void RenderDialog::slotRenderingFinished()
{
    accept();
}
void RenderDialog::slotFrameRendered(qreal time, int frameNumber)
{
    ui->lblFrame->setText(QString("%1 (%2 s)").arg(frameNumber).arg(time));
    ui->progressBar->setValue(frameNumber);
}


void RenderDialog::slotStartClicked()
{
    ui->bStart->setEnabled(false);
    ui->bStop->setEnabled(true);
    ui->comboFps->setEnabled(false);
    ui->comboOutputSize->setEnabled(false);
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
    float fps = ui->comboFps->currentText().toFloat();
    qDebug() << "Signal: fps changed to " << fps;
//    ui->progressBar->setMaximum((int) m_project->length()*fps);
    emit signalChangeFps(fps);
//    m_project->slotSetFps(ui->comboFps->currentText().toFloat());
}
void RenderDialog::slotRenderFrameSizeChanged()
{
    const FrameSize frameSize = (FrameSize)ui->comboOutputSize->itemData(ui->comboOutputSize->currentIndex()).toInt();
    emit signalChangeRenderFrameSize(frameSize);
//    m_project->slotSetRenderFrameSize(frameSize);
}

