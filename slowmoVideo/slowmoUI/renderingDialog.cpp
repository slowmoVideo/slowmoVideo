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
#include "../project/renderTask_sV.h"
#include "../project/imagesRenderTarget_sV.h"

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

    m_targetGroup = new QButtonGroup(this);
    m_targetGroup->addButton(ui->radioImages);
    m_targetGroup->addButton(ui->radioVideo);

    ui->cbSize->addItem("Original size", QVariant(FrameSize_Orig));
    ui->cbSize->addItem("Small", QVariant(FrameSize_Small));

    bool b = true;
    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->cbFps, SIGNAL(editTextChanged(QString)), this, SLOT(slotValidate()));
//    b &= connect(ui->cbFps, SIGNAL(currentIndexChanged(int)), this, SLOT(slotValidate())); // necessary?
    b &= connect(ui->imagesOutputDir, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    b &= connect(ui->imagesFilenamePattern, SIGNAL(textChanged(QString)), this, SLOT(slotValidate()));
    b &= connect(ui->bImagesBrowseDir, SIGNAL(clicked()), this, SLOT(slotBrowseImagesDir()));
    Q_ASSERT(b);

    slotValidate();
}

RenderingDialog::~RenderingDialog()
{
    delete m_targetGroup;
    delete ui;
}

RenderTask_sV* RenderingDialog::buildTask()
{
    if (slotValidate()) {
        RenderTask_sV *task = new RenderTask_sV(m_project);
        task->setFPS(ui->cbFps->currentText().toFloat());
        task->setSize((FrameSize)ui->cbSize->itemData(ui->cbSize->currentIndex()).toInt());
        ImagesRenderTarget_sV *renderTarget = new ImagesRenderTarget_sV();
        renderTarget->setFilenamePattern(ui->imagesFilenamePattern->text());
        renderTarget->setTargetDir(ui->imagesOutputDir->text());
        task->setRenderTarget(renderTarget);
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

    ui->bOk->setEnabled(ok);

    return ok;
}

void RenderingDialog::slotBrowseImagesDir()
{
    QFileDialog dialog(this, "Output directory for rendered images");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec() == QDialog::Accepted) {
        ui->imagesOutputDir->setText(dialog.selectedFiles().at(0));
    }
}
