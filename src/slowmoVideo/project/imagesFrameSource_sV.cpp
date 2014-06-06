/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "imagesFrameSource_sV.h"
#include "project_sV.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtGui/QImageReader>

ImagesFrameSource_sV::ImagesFrameSource_sV(Project_sV *project, QStringList images) throw(FrameSourceError) :
    AbstractFrameSource_sV(project),
    m_fps(24, 1),
    m_initialized(false),
    m_stopInitialization(false),
    m_nextFrame(0)
{
    QString msg = validateImages(images);
    if (msg.length() > 0) {
        throw FrameSourceError("Image frame source: " + msg);
    }

    m_imagesList.append(images);
    m_imagesList.sort();

    QImage repImage(m_imagesList.at(0));
    if (repImage.isNull()) {
        qDebug() << "Image is null: " << m_imagesList.at(0);
        qDebug() << "Supported image formats: " << QImageReader::supportedImageFormats();
        throw FrameSourceError(QString("Cannot read image: %1").arg(m_imagesList.at(0)));
    }
    m_sizeSmall = repImage.size();
    if (m_sizeSmall.isEmpty()) {
        throw FrameSourceError(QString("Image read from %1 is empty.").arg(m_imagesList.at(0)));
    }
    while (m_sizeSmall.width() > 600) {
        m_sizeSmall = m_sizeSmall/2;
    }

    createDirectories();
}

QString ImagesFrameSource_sV::validateImages(const QStringList images)
{
    if (images.size() == 0) { return tr("No images selected."); }

    QSize size = QImage(images.at(0)).size();
    for (int i = 0; i < images.length(); i++) {
        if (QImage(images.at(i)).size() != size) {
            return tr("Image %1 is not of the same size (%2) as the first image (%3).")
                    .arg(images.at(i)).arg(toString(QImage(images.at(i)).size())).arg(toString(size));
        }
    }
    return QString();
}

const QStringList ImagesFrameSource_sV::inputFiles() const
{
    return m_imagesList;
}

void ImagesFrameSource_sV::slotUpdateProjectDir()
{
    m_dirImagesSmall.rmdir(".");
    createDirectories();
}

void ImagesFrameSource_sV::createDirectories()
{
    m_dirImagesSmall = m_project->getDirectory("frames/imagesSmall");
}

void ImagesFrameSource_sV::initialize()
{
    m_stopInitialization = false;
    QMetaObject::invokeMethod(this, "slotContinueInitialization", Qt::QueuedConnection);
}
bool ImagesFrameSource_sV::initialized() const
{
    return m_initialized;
}

/// \todo What if image missing?
void ImagesFrameSource_sV::slotContinueInitialization()
{
    emit signalNextTask(tr("Creating preview images from the input images"), m_imagesList.size());
    for (; m_nextFrame < m_imagesList.size(); m_nextFrame++) {

        QString outputFile(framePath(m_nextFrame, FrameSize_Small));
        if (QFile(outputFile).exists()) {
            emit signalTaskItemDescription(tr("Resized image already exists for %1").arg(QFileInfo(m_imagesList.at(m_nextFrame)).fileName()));
        } else {
            emit signalTaskItemDescription(tr("Re-sizing image %1 to:\n%2")
                                           .arg(QFileInfo(m_imagesList.at(m_nextFrame)).fileName())
                                           .arg(outputFile));
            QImage small = QImage(m_imagesList.at(m_nextFrame)).scaled(m_sizeSmall, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            small.save(outputFile);
        }

        emit signalTaskProgress(m_nextFrame);
        if (m_stopInitialization) {
            break;
        }
    }
    m_initialized = true;
    emit signalAllTasksFinished();
}

void ImagesFrameSource_sV::slotAbortInitialization()
{
    m_stopInitialization = true;
}

int64_t ImagesFrameSource_sV::framesCount() const
{
    return m_imagesList.size();
}
const Fps_sV* ImagesFrameSource_sV::fps() const
{
    return &m_fps;
}

QImage ImagesFrameSource_sV::frameAt(const uint frame, const FrameSize frameSize)
{
    if (int(frame) < m_imagesList.size()) {
        return QImage(framePath(frame, frameSize));
    } else {
        return QImage();
    }
}
const QString ImagesFrameSource_sV::framePath(const uint frame, const FrameSize frameSize) const
{
    switch (frameSize) {
    case FrameSize_Orig:
        return QString(m_imagesList.at(frame));
    case FrameSize_Small:
    default:
        return QString(m_dirImagesSmall.absoluteFilePath(QFileInfo(m_imagesList.at(frame)).completeBaseName() + ".png"));
    }
}
