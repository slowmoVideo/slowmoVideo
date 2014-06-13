/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "newProjectDialog.h"
#include "ui_newProjectDialog.h"

#include "project/videoFrameSource_sV.h"
#include "project/imagesFrameSource_sV.h"


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

    ui->projectDir->setText(m_settings.value("directories/lastProjectDir", QDir::current().absolutePath()).toString());
    m_videoInfo.streamsCount = 0;

    connect(ui->browseInputVideo, SIGNAL(clicked()), this, SLOT(slotSelectVideoFile()));
    connect(ui->browseInputImages, SIGNAL(clicked()), this, SLOT(slotSelectImages()));
    connect(ui->browseProjectDir, SIGNAL(clicked()), this, SLOT(slotSelectProjectDir()));
    connect(ui->inputVideo, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateVideoInfo()));
    connect(ui->projectDir, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonStates()));
    connect(ui->projectFilename, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonStates()));

    connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));

    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateFrameSourceType()));

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

Project_sV* NewProjectDialog::buildProject() throw(FrameSourceError)
{
    Project_sV *project = new Project_sV(ui->projectDir->text());
    AbstractFrameSource_sV *frameSource = NULL;
    if (ui->radioVideo->isChecked()) {
        frameSource = new VideoFrameSource_sV(project, ui->inputVideo->text());
        m_settings.setValue("directories/lastInputVideo", QFileInfo(ui->inputVideo->text()).absolutePath());
    } else {
        frameSource = new ImagesFrameSource_sV(project, m_images);
        m_settings.setValue("directories/lastInputImage", QFileInfo(m_images.last()).absolutePath());
    }
    project->loadFrameSource(frameSource);

    m_settings.setValue("directories/lastProjectDir", ui->projectDir->text());
    m_settings.setValue("directories/lastProjectDir", ui->projectDir->text());

    return project;
}
const QString NewProjectDialog::projectFilename() const
{
    return QString(ui->projectDir->text() + "/" + ui->projectFilename->text() + ".sVproj");
}

void NewProjectDialog::slotSelectVideoFile()
{
    QFileDialog dialog(this, tr("Select input video file"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (ui->inputVideo->text().length() > 0) {
        dialog.setDirectory(ui->inputVideo->text());
    } else {
        dialog.setDirectory(m_settings.value("directories/lastInputVideo", QDir::homePath()).toString());
    }
    if (dialog.exec() == QDialog::Accepted) {
        ui->inputVideo->setText(dialog.selectedFiles().at(0));
        ui->txtVideoInfo->clear();

        slotUpdateVideoInfo();

        if (m_videoInfo.streamsCount <= 0) {
            // No video stream found. Check if the path contains a non-ASCII character and warn if this is the case.
            unsigned char ascii;
            for (int i = 0; i < ui->inputVideo->text().length(); i++) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                ascii = ui->inputVideo->text().at(i).toAscii();
#else
                ascii = ui->inputVideo->text().at(i).toLatin1();
#endif
                if (ascii == 0 || ascii > 0x7f) {
                    ui->txtVideoInfo->appendPlainText(
                                tr("Character %1 is not an ASCII character. This file path will likely not work with ffmpeg.")
                                .arg(ui->inputVideo->text().at(i)));
                    break;
                }
            }
        }
    }
}

void NewProjectDialog::slotSelectImages()
{
    QFileDialog dialog(this, tr("Select input images"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (m_images.size() > 0) {
        dialog.setDirectory(QFileInfo(m_images.last()).absolutePath());
    } else {
        dialog.setDirectory(m_settings.value("directories/lastInputImage", QDir::homePath()).toString());
    }
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
    QFileDialog dialog(this, tr("Select a project directory"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    if (ui->projectDir->text().length() > 0) {
        dialog.setDirectory(ui->projectDir->text());
    } else {
        dialog.setDirectory(m_settings.value("directories/lastProjectDir").toString());
    }
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
        QString text = trUtf8("Number of video streams: %1\nFrames: %2\nSize: %3×%4\n")
                .arg(m_videoInfo.streamsCount).arg(m_videoInfo.framesCount)
                .arg(m_videoInfo.width).arg(m_videoInfo.height);
        text.append(tr("Frame rate: %1/%2").arg(m_videoInfo.frameRateNum).arg(m_videoInfo.frameRateDen));
        ui->txtVideoInfo->setPlainText(text);
    } else {
        m_videoInfo.streamsCount = 0;
        ui->txtVideoInfo->setPlainText(tr("No video stream detected."));
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
        if (m_videoInfo.streamsCount > 0 && m_videoInfo.framesCount > 0) {
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
            ui->txtImageInfo->setText(tr("Image size: %1").arg(toString(QImage(m_images.at(0)).size())));
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
