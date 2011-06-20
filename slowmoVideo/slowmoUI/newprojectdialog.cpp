/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"

#include "../project/videoFrameSource_sV.h"
#include "../project/imagesFrameSource_sV.h"


#include <QDebug>

#include <QtCore/QFile>
#include <QtCore/QDir>

#include <QFileDialog>
#include <QButtonGroup>

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->radioVideo);
    m_buttonGroup->addButton(ui->radioImages);
    ui->radioVideo->setChecked(true);

    m_videoInfo.streamsCount = 0;

    bool b = true;
    b &= connect(ui->browseInputVideo, SIGNAL(clicked()), this, SLOT(slotSelectVideoFile()));
    b &= connect(ui->browseInputImages, SIGNAL(clicked()), this, SLOT(slotSelectImages()));
    b &= connect(ui->browseProjectDir, SIGNAL(clicked()), this, SLOT(slotSelectProjectDir()));
    b &= connect(ui->inputVideo, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateVideoInfo()));
    b &= connect(ui->projectDir, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonStates()));
    b &= connect(ui->projectFilename, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonStates()));

    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));

    b &= connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateFrameSourceType()));
    Q_ASSERT(b);

    slotUpdateImagesInfo();
    slotUpdateVideoInfo();
    slotUpdateButtonStates();
    slotUpdateFrameSourceType();
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
    delete m_buttonGroup;
}

Project_sV* NewProjectDialog::buildProject()
{
    Project_sV *project = new Project_sV(ui->projectDir->text());
    AbstractFrameSource_sV *frameSource = NULL;
    if (ui->radioVideo->isChecked()) {
        frameSource = new VideoFrameSource_sV(project, ui->inputVideo->text());
    } else {
        frameSource = new ImagesFrameSource_sV(project, m_images);
    }
    project->loadFrameSource(frameSource);
    return project;
}
const QString NewProjectDialog::projectFilename() const
{
    return QString(ui->projectDir->text() + "/" + ui->projectFilename->text() + ".sVproj");
}

void NewProjectDialog::slotSelectVideoFile()
{
    QFileDialog dialog(this, "Select input video file");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec() == QDialog::Accepted) {
        ui->inputVideo->setText(dialog.selectedFiles().at(0));
        ui->txtVideoInfo->clear();

        slotUpdateVideoInfo();
    }
}

void NewProjectDialog::slotSelectImages()
{
    QFileDialog dialog(this, "Select input images");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (dialog.exec() == QDialog::Accepted) {

        m_images = dialog.selectedFiles();

        ui->inputImages->clear();
        for (int i = 0; i < m_images.size(); i++) {
            new QListWidgetItem(m_images.at(i), ui->inputImages);
        }

        slotUpdateImagesInfo();
    }
}

void NewProjectDialog::slotSelectProjectDir()
{
    QFileDialog dialog(this, "Select a project directory");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec() == QDialog::Accepted) {
        ui->projectDir->setText(dialog.selectedFiles().at(0));
        slotUpdateButtonStates();
    }
}

void NewProjectDialog::slotUpdateVideoInfo()
{
    QFile file(ui->inputVideo->text());
    if (file.exists()) {
        m_videoInfo = getInfo(ui->inputVideo->text().toStdString().c_str());
        QString text = QString::fromUtf8("Number of video streams: %1\nFrames: %2\nSize: %3×%4\n")
                .arg(m_videoInfo.streamsCount).arg(m_videoInfo.framesCount)
                .arg(m_videoInfo.width).arg(m_videoInfo.height);
        text.append(QString("Frame rate: %1/%2").arg(m_videoInfo.frameRateNum).arg(m_videoInfo.frameRateDen));
        ui->txtVideoInfo->setPlainText(text);
    } else {
        m_videoInfo.streamsCount = 0;
        ui->txtVideoInfo->setPlainText("No video stream detected.");
    }
    slotUpdateButtonStates();
}

void NewProjectDialog::slotUpdateImagesInfo()
{
    m_imagesMsg = ImagesFrameSource_sV::validateImages(m_images);
    slotUpdateButtonStates();
}

void NewProjectDialog::slotUpdateButtonStates()
{
    bool ok = true;

    if (ui->projectDir->text().length() > 0) {
        QDir dir(ui->projectDir->text());
        ui->cbDirectoryCreated->setChecked(!dir.exists());
        ui->projectDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        m_projectDir = ui->projectDir->text();
    } else {
        ui->projectDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        ok = false;
    }
    QFile projectFile(projectFilename());
    if (ui->projectFilename->text().length() > 0 && !projectFile.exists()) {
        ui->projectFilename->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
    } else {
        ui->projectFilename->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        ok = false;
    }

    if (ui->radioVideo->isChecked()) {
        // Validate the video file
        if (m_videoInfo.streamsCount > 0) {
            ui->inputVideo->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
            m_inputFile = ui->inputVideo->text();
        } else {
            ui->inputVideo->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
            ok = false;
        }
    } else if (ui->radioImages->isChecked()) {
        // Validate the images
        if (m_imagesMsg.length() == 0) {
            ui->inputImages->setStyleSheet(QString("QListWidget { background-color: %1; }").arg(Colours_sV::colOk.name()));
            ui->txtImageInfo->setText("Image size: " + toString(QImage(m_images.at(0)).size()));
        } else {
            ui->inputImages->setStyleSheet(QString("QListWidget { background-color: %1; }").arg(Colours_sV::colBad.name()));
            ui->txtImageInfo->setText(m_imagesMsg);
            ok = false;
        }
    }

    ui->bOk->setEnabled(ok);
}

void NewProjectDialog::slotUpdateFrameSourceType()
{
    ui->groupImages->setEnabled(ui->radioImages->isChecked());
    ui->groupImages->setVisible(ui->radioImages->isChecked());
    ui->groupVideo->setEnabled(ui->radioVideo->isChecked());
    ui->groupVideo->setVisible(ui->radioVideo->isChecked());
    slotUpdateButtonStates();
    QSize prevSize = size();
    adjustSize();
    resize(prevSize.width(), size().height());
}
