/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "config.h"

#include "renderingDialog.h"
#include "ui_renderingDialog.h"

#include "lib/defs_sV.hpp"
#include "project/motionBlur_sV.h"
#include "project/project_sV.h"
#include "project/projectPreferences_sV.h"
#include "project/renderTask_sV.h"
#include "project/imagesRenderTarget_sV.h"
#include "project/abstractFlowSource_sV.h"
#include "project/flowSourceOpenCV_sV.h"
#include "project/flowSourceV3D_sV.h"

#ifdef USE_FFMPEG
#if 0
#include "project/new_videoRenderTarget.h"
#else
#include "project/exportVideoRenderTarget.h"
#endif
#else
#include "project/videoRenderTarget_sV.h"
#endif
#include "project/emptyFrameSource_sV.h"

#include <QButtonGroup>
#include <QFileDialog>
#include <QSettings> // TODO: better
#include <QMessageBox>
#include <QComboBox>

RenderingDialog::RenderingDialog(Project_sV *project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenderingDialog),
    m_project(project)
{
    ui->setupUi(this);

    // Render section
    m_sectionGroup = new QButtonGroup(this);
    m_sectionGroup->addButton(ui->radioFullProject);
    m_sectionGroup->addButton(ui->radioSection);
    m_sectionGroup->addButton(ui->radioTagSection);
    QString mode(m_project->preferences()->renderSectionMode());
    if (mode == "full") {
        ui->radioFullProject->setChecked(true);
    } else if (mode == "expr") {
        ui->radioSection->setChecked(true);
    } else if (mode == "tags") {
        ui->radioTagSection->setChecked(true);
    } else {
        qDebug() << "Unknown render section mode: " << mode;
        Q_ASSERT(false);
    }

    // Optical flow
    ui->lambda->setValue(m_project->preferences()->flowV3DLambda());

    QSettings settings; //TODO: better define in project ?
    ui->opticalFlowAlgo->clear();
    QString flow_method = settings.value("preferences/flowMethod", "OpenCV-CPU").toString();
    if (flow_method == "V3D") {
        ui->opticalFlowAlgo->addItem(tr("flowBuilder"), QVariant(1));
    } else {
        ui->opticalFlowAlgo->addItem(tr("OpenCV - Farneback"), QVariant(2));
        ui->opticalFlowAlgo->addItem(tr("OpenCV - Dual TVL1"), QVariant(3));
    }

    connect(ui->opticalFlowAlgo, SIGNAL(activated(int)),
            ui->flowStackedWidget, SLOT(setCurrentIndex(int)));

    QWidget *flowbuilder_pane = ui->flowStackedWidget->widget(0);
    QWidget *farneback_pane = ui->flowStackedWidget->widget(1);
    QWidget *tvl1_pane = ui->flowStackedWidget->widget(2);
    if (flow_method == "V3D") {
        ui->opticalFlowAlgo->setCurrentIndex(0);
        ui->flowStackedWidget->setCurrentIndex(0);
        ui->flowStackedWidget->removeWidget(farneback_pane);
        ui->flowStackedWidget->removeWidget(tvl1_pane);
    } else {
        int algo = settings.value("preferences/preferredOpenCVAlgo", 0).toInt();
        ui->opticalFlowAlgo->setCurrentIndex(algo);
        ui->flowStackedWidget->setCurrentIndex(algo+1);
        ui->flowStackedWidget->removeWidget(flowbuilder_pane);
    }
    connect(ui->clearflow, SIGNAL(clicked()), this, SLOT(slotClearFlowCache()));
    // Motion blur
    ui->maxSamples->setValue(m_project->motionBlur()->maxSamples());
    ui->slowmoSamples->setValue(m_project->motionBlur()->slowmoSamples());
    m_blurGroup = new QButtonGroup(this);
    m_blurGroup->addButton(ui->radioBlurConvolution);
    m_blurGroup->addButton(ui->radioBlurStacking);
    m_blurGroup->addButton(ui->radioBlurNearest);
    if (m_project->preferences()->renderMotionblurType() == MotionblurType_Convolving) {
        ui->radioBlurConvolution->setChecked(true);
    } else if (m_project->preferences()->renderMotionblurType() == MotionblurType_Stacking) {
        ui->radioBlurStacking->setChecked(true);
    } else {
        ui->radioBlurNearest->setChecked(true);
    }

    fillTagLists();

    // Output target type
    m_targetGroup = new QButtonGroup(this);
    m_targetGroup->addButton(ui->radioImages);
    m_targetGroup->addButton(ui->radioVideo);
    if (m_project->preferences()->renderTarget() == "images") {
        ui->radioImages->setChecked(true);
    } else {
        ui->radioVideo->setChecked(true);
    }

    // Output target files
    ui->imagesOutputDir->setText(m_project->preferences()->imagesOutputDir());
    ui->imagesFilenamePattern->setText(m_project->preferences()->imagesFilenamePattern());
    ui->videoOutputFile->setText(m_project->preferences()->videoFilename());
    ui->vcodec->setText(m_project->preferences()->videoCodec());

    // FPS
    QString fps = QVariant(m_project->preferences()->renderFPS().fps()).toString();
    if (ui->cbFps->findText(fps) < 0 && fps.toFloat() > 0) {
        ui->cbFps->addItem(fps);
    }
    ui->cbFps->setCurrentIndex(ui->cbFps->findText(fps));

    // Output size
    ui->cbSize->addItem(tr("Original size"), QVariant(FrameSize_Orig));
    ui->cbSize->addItem(tr("Small"), QVariant(FrameSize_Small));
    ui->cbSize->setCurrentIndex(ui->cbSize->findData(QVariant(m_project->preferences()->renderFrameSize())));

    // Interpolation type
    ui->cbInterpolation->addItem(toString(InterpolationType_Forward), QVariant(InterpolationType_Forward));
    ui->cbInterpolation->addItem(toString(InterpolationType_ForwardNew), QVariant(InterpolationType_ForwardNew));
    ui->cbInterpolation->addItem(toString(InterpolationType_Twoway), QVariant(InterpolationType_Twoway));
    ui->cbInterpolation->addItem(toString(InterpolationType_TwowayNew), QVariant(InterpolationType_TwowayNew));
    ui->cbInterpolation->addItem(toString(InterpolationType_Bezier), QVariant(InterpolationType_Bezier));
    ui->cbInterpolation->addItem(toString(InterpolationType_None), QVariant(InterpolationType_None));
    ui->cbInterpolation->addItem(toString(InterpolationType_Nearest), QVariant(InterpolationType_Nearest));
    if (ui->cbInterpolation->findData(QVariant(m_project->preferences()->renderInterpolationType())) >= 0) {
        ui->cbInterpolation->setCurrentIndex(ui->cbInterpolation->findData(QVariant(m_project->preferences()->renderInterpolationType())));
    }

    connect(m_targetGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateRenderTarget()));
    connect(m_sectionGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotSectionModeChanged()));
    connect(ui->timeStart, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    connect(ui->timeEnd, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));

    connect(ui->cbStartTag, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTagIndexChanged()));
    connect(ui->cbEndTag, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTagIndexChanged()));

    connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->bSave, SIGNAL(clicked()), this, SLOT(slotSaveSettings()));

    connect(ui->cbFps, SIGNAL(editTextChanged(QString)), this, SLOT(slotValidate()));

    connect(ui->imagesOutputDir, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    connect(ui->imagesFilenamePattern, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    connect(ui->videoOutputFile, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    connect(ui->bImagesBrowseDir, SIGNAL(clicked()), this, SLOT(slotBrowseImagesDir()));
    connect(ui->bBrowseVideoOutputFile, SIGNAL(clicked()), this, SLOT(slotBrowseVideoFile()));

    // Restore rendering start/end
    int index = ui->cbStartTag->findText(m_project->preferences()->renderStartTag());
    if (index >= 0) {
        ui->cbStartTag->setCurrentIndex(index);
    }
    index = ui->cbEndTag->findText(m_project->preferences()->renderEndTag());
    if (index >= 0) {
        ui->cbEndTag->setCurrentIndex(index);
    }
    if (m_project->preferences()->renderStartTime().length() > 0) {
        ui->timeStart->setText(m_project->preferences()->renderStartTime());
    }
    if (m_project->preferences()->renderEndTime().length() > 0) {
        ui->timeEnd->setText(m_project->preferences()->renderEndTime());
    }

#if QT_VERSION >= 0x040700
    ui->timeStart->setPlaceholderText(QVariant(m_project->nodes()->startTime()).toString());
    ui->timeEnd->setPlaceholderText(QVariant(m_project->nodes()->endTime()).toString());
#endif

#ifndef USE_QTKIT
     ui->use_qt->setChecked(false);
     ui->use_qt->setEnabled(false);
#endif

    slotUpdateRenderTarget();
    slotSectionModeChanged();
}

RenderingDialog::~RenderingDialog()
{
    delete m_targetGroup;
    delete ui;
}

RenderTask_sV* RenderingDialog::buildTask()
{
    if (!slotValidate()) {
        return NULL;
    }
    slotSaveSettings();

    ProjectPreferences_sV *prefs = m_project->preferences();

    const QString imagesOutputDir = ui->imagesOutputDir->text();
    const QString imagesFilenamePattern = ui->imagesFilenamePattern->text();

    RenderTask_sV *task = new RenderTask_sV(m_project);
    task->renderPreferences().setFps(prefs->renderFPS());
    task->renderPreferences().size = prefs->renderFrameSize();
    task->renderPreferences().interpolation = prefs->renderInterpolationType();
    task->renderPreferences().motionblur = prefs->renderMotionblurType();


    if (ui->radioImages->isChecked()) {
        ImagesRenderTarget_sV *renderTarget = new ImagesRenderTarget_sV(task);
        renderTarget->setFilenamePattern(imagesFilenamePattern);
        renderTarget->setTargetDir(imagesOutputDir);
        task->setRenderTarget(renderTarget);
    } else if (ui->radioVideo->isChecked()) {
#ifdef USE_FFMPEG
#if 0
        newVideoRenderTarget *renderTarget = new newVideoRenderTarget(task);
#else
        exportVideoRenderTarget *renderTarget = new exportVideoRenderTarget(task);
#endif
        const bool use_qt = ui->use_qt->isChecked();
        if (!use_qt) {
            qDebug() << "using classical FFMPEG";
            renderTarget->setQT(0);
        }
#else
#warning "should not use this"
        VideoRenderTarget_sV *renderTarget = new VideoRenderTarget_sV(task);
#endif
        // check if file exist
        QFile filetest(ui->videoOutputFile->text());
        if (filetest.exists()) {
            int r = QMessageBox::warning(this, tr("slowmoUI"),
                tr("The file already exist.\n"
                "Do you want to overwrite it ?"),
                QMessageBox::Yes | QMessageBox::No);
            if (r == QMessageBox::Yes) {
            filetest.remove();
            } else {
            //TODO:  maybe should delete task ?
            return 0;
            }
        }
        renderTarget->setTargetFile(ui->videoOutputFile->text());
        renderTarget->setVcodec(ui->vcodec->text());
        task->setRenderTarget(renderTarget);
    } else {
        qDebug() << "Render target is neither images nor video. Not implemented?";
        Q_ASSERT(false);
    }

    if (ui->radioTagSection->isChecked()) {
        bool b;
        qreal start = ui->cbStartTag->itemData(ui->cbStartTag->currentIndex()).toFloat(&b);
        Q_ASSERT(b);
        qreal end = ui->cbEndTag->itemData(ui->cbEndTag->currentIndex()).toFloat(&b);
        Q_ASSERT(b);
        qDebug() << QString("Rendering tag section from %1 (%2) to %3 (%4)")
                    .arg(ui->cbStartTag->currentText())
                    .arg(start).arg(ui->cbEndTag->currentText()).arg(end);
        Q_ASSERT(start <= end);
        task->setTimeRange(start, end);
    } else if (ui->radioSection->isChecked()) {
        qDebug() << QString("Rendering time section from %1 to %3")
                    .arg(ui->cbStartTag->currentText())
                    .arg(ui->cbEndTag->currentText());
        task->setTimeRange(ui->timeStart->text(), ui->timeEnd->text());
    }

    QString mode;
    if (ui->radioFullProject->isChecked()) {
        mode = "full";
    } else if (ui->radioSection->isChecked()) {
        mode = "time";
        m_project->preferences()->renderStartTime() = ui->timeStart->text();
        m_project->preferences()->renderEndTime() = ui->timeEnd->text();
    } else if (ui->radioTagSection->isChecked()) {
        mode = "tags";
        m_project->preferences()->renderStartTag() = ui->cbStartTag->currentText();
        m_project->preferences()->renderEndTag() = ui->cbEndTag->currentText();
    } else {
        qDebug() << "No section mode selected?";
        Q_ASSERT(false);
    }

    // set optical flow parameters
    QSettings settings;
    QString flow_method = settings.value("preferences/flowMethod", "OpenCV-CPU").toString();
    if (flow_method == "V3D") {
        AbstractFlowSource_sV *flow_algo = m_project->flowSource();
        flow_algo->setLambda(prefs->flowV3DLambda());
    }
    else if (flow_method == "OpenCV-CPU" || flow_method == "OpenCV-OCL") {
        int algo_index = ui->opticalFlowAlgo->currentIndex();
        qDebug() << "algo index is " << algo_index;
        FlowSourceOpenCV_sV *flow_algo = (FlowSourceOpenCV_sV *)m_project->flowSource();

        switch (algo_index) {
            case 0:
                flow_algo->setupOpticalFlow(
                    ui->FarnLevels->value(),
                    ui->FarnWin->value(),
                    ui->FarnPoly->value(),
                    ui->FarnPyr->value(),
                    ui->FarnPolyN->value()
                );
                break;

            case 1:
                flow_algo->setupTVL1(
                    ui->TVLtau->value(),
                    ui->TVLlambda->value(),
                    ui->TVLnscales->value(),
                    ui->TVLwarps->value(),
                    ui->TVLiterations->value(),
                    ui->TVLepsilon->value()
                );
                break;

            default:
                qDebug() << "no algo defined";
        }
    }
    else {
        throw Error_sV("Unsupported Flow method");
    }
    return task;
}

void RenderingDialog::fillTagLists()
{
    QList<Tag_sV> list;
    for (int i = 0; i < m_project->tags()->size(); i++) {
        if (m_project->tags()->at(i).axis() == TagAxis_Output
                && m_project->tags()->at(i).time() > m_project->nodes()->startTime()
                && m_project->tags()->at(i).time() < m_project->nodes()->endTime()) {
            list << m_project->tags()->at(i);
        }
    }
    qSort(list);
    ui->cbStartTag->addItem(tr("<Start>"), QVariant(m_project->nodes()->startTime()));
    for (int i = 0; i < list.size(); i++) {
        ui->cbStartTag->addItem(list.at(i).description(), QVariant(list.at(i).time()));
        ui->cbEndTag->addItem(list.at(i).description(), QVariant(list.at(i).time()));
    }
    ui->cbEndTag->addItem(tr("<End>"), QVariant(m_project->nodes()->endTime()));
}

void RenderingDialog::slotSaveSettings()
{
    qDebug() << "RenderingDialog::slotSaveSettings()";

    const InterpolationType interpolation = (InterpolationType)ui->cbInterpolation->itemData(ui->cbInterpolation->currentIndex()).toInt();
    const FrameSize size = (FrameSize)ui->cbSize->itemData(ui->cbSize->currentIndex()).toInt();
    const QString imagesOutputDir = ui->imagesOutputDir->text();
    const QString imagesFilenamePattern = ui->imagesFilenamePattern->text();
    const float fps = ui->cbFps->currentText().toFloat();

    const bool use_qt = ui->use_qt->isChecked();

    m_project->motionBlur()->setMaxSamples(ui->maxSamples->value());
    m_project->motionBlur()->setSlowmoSamples(ui->slowmoSamples->value());
    m_project->preferences()->flowV3DLambda() = ui->lambda->value();

    if (ui->radioBlurConvolution->isChecked()) {
        m_project->preferences()->renderMotionblurType() = MotionblurType_Convolving;
    } else if (ui->radioBlurStacking->isChecked()) {
        m_project->preferences()->renderMotionblurType() = MotionblurType_Stacking;
    } else {
        m_project->preferences()->renderMotionblurType() = MotionblurType_Nearest;
    }

    QString mode;
    if (ui->radioFullProject->isChecked()) {
        mode = "full";
    } else if (ui->radioSection->isChecked()) {
        mode = "expr";
        m_project->preferences()->renderStartTime() = ui->timeStart->text();
        m_project->preferences()->renderEndTime() = ui->timeEnd->text();
    } else if (ui->radioTagSection->isChecked()) {
        mode = "tags";
        m_project->preferences()->renderStartTag() = ui->cbStartTag->currentText();
        m_project->preferences()->renderEndTag() = ui->cbEndTag->currentText();
    } else {
        qDebug() << "No section mode selected?";
        Q_ASSERT(false);
    }
    m_project->preferences()->renderSectionMode() = mode;
    m_project->preferences()->imagesOutputDir() = imagesOutputDir;
    m_project->preferences()->imagesFilenamePattern() = imagesFilenamePattern;
    m_project->preferences()->videoFilename() = ui->videoOutputFile->text();
    m_project->preferences()->videoCodec() = ui->vcodec->text();
    m_project->preferences()->renderInterpolationType() = interpolation;
    m_project->preferences()->renderFrameSize() = size;
    m_project->preferences()->renderFPS() = fps;
    m_project->preferences()->renderTarget() = ui->radioImages->isChecked() ? "images" : "video";
    m_project->preferences()->renderFormat() = use_qt;

    accept();
}

bool RenderingDialog::slotValidate()
{
    qDebug() << "RenderingDialog::slotValidate()";

    bool ok = true;

    float fps = ui->cbFps->currentText().toFloat(&ok);
    ok &= fps > 0;
    if (ok) {
        ui->cbFps->setStyleSheet(QString("QComboBox { background-color: %1; }").arg(Colours_sV::colOk.name()));
    } else {
        ui->cbFps->setStyleSheet(QString("QComboBox { background-color: %1; }").arg(Colours_sV::colBad.name()));
    }

    if (ui->radioImages->isChecked()) {
        if (ui->imagesFilenamePattern->text().contains("%1")) {
            ui->imagesFilenamePattern->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        } else {
            ok = false;
            ui->imagesFilenamePattern->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        }

        if (ui->imagesOutputDir->text().length() > 0) {
            ui->imagesOutputDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        } else {
            ok = false;
            ui->imagesOutputDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        }
    } else if (ui->radioVideo->isChecked()) {
        if (ui->videoOutputFile->text().length() > 0) {
            ui->videoOutputFile->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        } else {
            ok = false;
            ui->videoOutputFile->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        }
    } else {
        Q_ASSERT(false);
    }

    if (ui->radioSection->isChecked()) {
        bool startOk = false;
        bool endOk = false;
        qreal timeStart = 0;
        qreal timeEnd = 0;
        QStringList messages;

        Fps_sV currentFps(ui->cbFps->currentText().toFloat());
        try {
            timeStart = m_project->toOutTime(ui->timeStart->text(), currentFps);
            startOk = true;
        } catch (Error_sV &err) {
            messages << err.message();
        }
        try {
            timeEnd = m_project->toOutTime(ui->timeEnd->text(), currentFps);
            endOk = true;
        } catch (Error_sV &err) {
            messages << err.message();
        }
        if (timeEnd <= timeStart) {
            endOk = false;
            messages << tr("Start time must be < end time!");
        }

        messages << tr("Rendering from %1 s to %2 s.").arg(timeStart).arg(timeEnd);
        ui->sectionMessage->setText(messages.join("\n"));

        ok &= startOk && endOk;

        if (!startOk) {
            ui->timeStart->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        } else {
            ui->timeStart->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        }
        if (!endOk) {
            ui->timeEnd->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        } else {
            ui->timeEnd->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        }
    }

    ok &= dynamic_cast<EmptyFrameSource_sV*>(m_project->frameSource()) == NULL;

    ok &= m_project->nodes()->size() >= 2;

    ui->bOk->setEnabled(ok);

    return ok;
}

void RenderingDialog::slotUpdateRenderTarget()
{
    ui->groupImages->setVisible(ui->radioImages->isChecked());
    ui->groupVideo->setVisible(ui->radioVideo->isChecked());
    slotValidate();
}

void RenderingDialog::slotBrowseImagesDir()
{
    QFileDialog dialog(this, tr("Output directory for rendered images"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setDirectory(ui->imagesOutputDir->text());
    if (dialog.exec() == QDialog::Accepted) {
        ui->imagesOutputDir->setText(dialog.selectedFiles().at(0));
    }
}

void RenderingDialog::slotBrowseVideoFile()
{
    QFileDialog dialog(this, tr("Output video file"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(QFileInfo(ui->videoOutputFile->text()).absolutePath());
    if (dialog.exec() == QDialog::Accepted) {
        ui->videoOutputFile->setText(dialog.selectedFiles().at(0));
    }
}

void RenderingDialog::slotSectionModeChanged()
{
    ui->timeStart->setVisible(ui->radioSection->isChecked());
    ui->timeEnd->setVisible(ui->radioSection->isChecked());
    ui->sectionMessage->setVisible(ui->radioSection->isChecked());
    ui->cbStartTag->setVisible(ui->radioTagSection->isChecked());
    ui->cbEndTag->setVisible(ui->radioTagSection->isChecked());
    ui->lblcTo->setVisible(ui->radioSection->isChecked() || ui->radioTagSection->isChecked());
    slotValidate();
}

void RenderingDialog::slotTagIndexChanged()
{
    if (QObject::sender() == ui->cbStartTag) {
        qDebug() << "Start tag";
        if (ui->cbEndTag->currentIndex() < ui->cbStartTag->currentIndex()) {
            ui->cbEndTag->setCurrentIndex(ui->cbStartTag->currentIndex());
        }
    } else {
        qDebug() << "End tag";
        if (ui->cbStartTag->currentIndex() > ui->cbEndTag->currentIndex()) {
            ui->cbStartTag->setCurrentIndex(ui->cbEndTag->currentIndex());
        }
    }
}

#if 0
void MainWindow::comboBox_Activated()
{
  std::cout << "Activated " << this->ui.comboBox->currentIndex() << std::endl;
}
#endif

void RenderingDialog::slotClearFlowCache() {
    m_project->flowSource()->clearFlowCache();
}



