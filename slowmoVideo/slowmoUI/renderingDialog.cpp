/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "renderingDialog.h"
#include "ui_renderingDialog.h"

#include "../lib/defs_sV.hpp"
#include "../project/project_sV.h"
#include "../project/projectPreferences_sV.h"
#include "../project/renderTask_sV.h"
#include "../project/imagesRenderTarget_sV.h"
#include "../project/videoRenderTarget_sV.h"
#include "../project/emptyFrameSource_sV.h"

#include <QButtonGroup>
#include <QFileDialog>

RenderingDialog::RenderingDialog(Project_sV *project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenderingDialog),
    m_project(project)
{
    ui->setupUi(this);

    ui->radioSection->setEnabled(false);
    ui->timeStart->setEnabled(false);
    ui->timeEnd->setEnabled(false);

    ui->imagesOutputDir->setText(m_project->preferences()->imagesOutputDir());
    ui->imagesFilenamePattern->setText(m_project->preferences()->imagesFilenamePattern());
    ui->videoOutputFile->setText(m_project->preferences()->videoFilename());
    ui->vcodec->setText(m_project->preferences()->videoCodec());

    QString fps = QVariant(m_project->preferences()->renderFPS()).toString();
    if (ui->cbFps->findText(fps) < 0 && fps.toFloat() > 0) {
        ui->cbFps->addItem(fps);
    }
    ui->cbFps->setCurrentIndex(ui->cbFps->findText(fps));

    m_targetGroup = new QButtonGroup(this);
    m_targetGroup->addButton(ui->radioImages);
    m_targetGroup->addButton(ui->radioVideo);
    if (m_project->preferences()->renderTarget() == "images") {
        ui->radioImages->setChecked(true);
    } else {
        ui->radioVideo->setChecked(true);
    }

    ui->cbSize->addItem("Original size", QVariant(FrameSize_Orig));
    ui->cbSize->addItem("Small", QVariant(FrameSize_Small));
    ui->cbSize->setCurrentIndex(ui->cbSize->findData(QVariant(m_project->preferences()->renderFrameSize())));

    ui->cbInterpolation->addItem(toString(InterpolationType_Forward), QVariant(InterpolationType_Forward));
    ui->cbInterpolation->addItem(toString(InterpolationType_ForwardNew), QVariant(InterpolationType_ForwardNew));
    ui->cbInterpolation->addItem(toString(InterpolationType_Twoway), QVariant(InterpolationType_Twoway));
    ui->cbInterpolation->addItem(toString(InterpolationType_TwowayNew), QVariant(InterpolationType_TwowayNew));
    ui->cbInterpolation->addItem(toString(InterpolationType_Bezier), QVariant(InterpolationType_Bezier));
    if (ui->cbInterpolation->findData(QVariant(m_project->preferences()->renderInterpolationType())) >= 0) {
        ui->cbInterpolation->setCurrentIndex(ui->cbInterpolation->findData(QVariant(m_project->preferences()->renderInterpolationType())));
    }

    bool b = true;
    b &= connect(m_targetGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateRenderTarget()));

    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));

    b &= connect(ui->cbFps, SIGNAL(editTextChanged(QString)), this, SLOT(slotValidate()));
//    b &= connect(ui->cbFps, SIGNAL(currentIndexChanged(int)), this, SLOT(slotValidate())); // necessary?

    b &= connect(ui->imagesOutputDir, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    b &= connect(ui->imagesFilenamePattern, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    b &= connect(ui->videoOutputFile, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));

    b &= connect(ui->bImagesBrowseDir, SIGNAL(clicked()), this, SLOT(slotBrowseImagesDir()));
    b &= connect(ui->bBrowseVideoOutputFile, SIGNAL(clicked()), this, SLOT(slotBrowseVideoFile()));
    Q_ASSERT(b);

    slotUpdateRenderTarget();
}

RenderingDialog::~RenderingDialog()
{
    delete m_targetGroup;
    delete ui;
}

RenderTask_sV* RenderingDialog::buildTask()
{
    if (slotValidate()) {
        const InterpolationType interpolation = (InterpolationType)ui->cbInterpolation->itemData(ui->cbInterpolation->currentIndex()).toInt();
        const FrameSize size = (FrameSize)ui->cbSize->itemData(ui->cbSize->currentIndex()).toInt();
        const QString imagesOutputDir = ui->imagesOutputDir->text();
        const QString imagesFilenamePattern = ui->imagesFilenamePattern->text();
        const float fps = ui->cbFps->currentText().toFloat();

        RenderTask_sV *task = new RenderTask_sV(m_project);
        task->setFPS(fps);
        task->setSize(size);
        task->setInterpolationType(interpolation);

        if (ui->radioImages->isChecked()) {
            ImagesRenderTarget_sV *renderTarget = new ImagesRenderTarget_sV(task);
            renderTarget->setFilenamePattern(imagesFilenamePattern);
            renderTarget->setTargetDir(imagesOutputDir);
            task->setRenderTarget(renderTarget);
        } else if (ui->radioVideo->isChecked()) {
            VideoRenderTarget_sV *renderTarget = new VideoRenderTarget_sV(task);
            renderTarget->setTargetFile(ui->videoOutputFile->text());
            renderTarget->setVcodec(ui->vcodec->text());
            task->setRenderTarget(renderTarget);
        } else {
            Q_ASSERT(false);
        }

        m_project->preferences()->imagesOutputDir() = imagesOutputDir;
        m_project->preferences()->imagesFilenamePattern() = imagesFilenamePattern;
        m_project->preferences()->videoFilename() = ui->videoOutputFile->text();
        m_project->preferences()->videoCodec() = ui->vcodec->text();
        m_project->preferences()->renderInterpolationType() = interpolation;
        m_project->preferences()->renderFrameSize() = size;
        m_project->preferences()->renderFPS() = fps;
        m_project->preferences()->renderTarget() = ui->radioImages->isChecked() ? "images" : "video";
        return task;
    } else {
        return NULL;
    }
}

bool RenderingDialog::slotValidate()
{
    bool ok = true;

    ui->cbFps->currentText().toFloat(&ok);
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

    ok &= dynamic_cast<EmptyFrameSource_sV*>(m_project->frameSource()) == NULL;

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
    QFileDialog dialog(this, "Output directory for rendered images");
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
    QFileDialog dialog(this, "Output video file");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(QFileInfo(ui->videoOutputFile->text()).absolutePath());
    if (dialog.exec() == QDialog::Accepted) {
        ui->videoOutputFile->setText(dialog.selectedFiles().at(0));
    }
}
