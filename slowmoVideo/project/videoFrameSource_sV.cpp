/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "videoFrameSource_sV.h"
#include "project_sV.h"

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QRegExp>

QRegExp VideoFrameSource_sV::regexFrameNumber("frame=\\s*(\\d+)");

VideoFrameSource_sV::VideoFrameSource_sV(const Project_sV *project, const QString &filename) throw(NoVideoStreamsException) :
    AbstractFrameSource_sV(project),
    m_inFile(filename),
    m_ffmpeg(NULL)
{
    m_videoInfo = new VideoInfoSV();

    *m_videoInfo = getInfo(filename.toStdString().c_str());
    if (m_videoInfo->streamsCount <= 0) {
        qDebug() << "Video info is invalid: " << filename;
        throw NoVideoStreamsException();
    }

    m_dirFramesSmall = project->getDirectory("framesSmall");
    m_dirFramesOrig = project->getDirectory("framesOrig");

    m_timer = new QTimer(this);

    bool b = true;
    b &= connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressUpdate()));
    Q_ASSERT(b);

    // Start the frame extraction process
    slotExtractOrigFrames();
}
VideoFrameSource_sV::~VideoFrameSource_sV()
{
    if (m_ffmpeg != NULL) { delete m_ffmpeg; }
    delete m_timer;
    delete m_videoInfo;
}

int64_t VideoFrameSource_sV::framesCount() const
{
    return m_videoInfo->framesCount;
}
int VideoFrameSource_sV::frameRateNum() const
{
    return m_videoInfo->frameRateNum;
}
int VideoFrameSource_sV::frameRateDen() const
{
    return m_videoInfo->frameRateDen;
}
QImage VideoFrameSource_sV::frameAt(const uint frame, const FrameSize frameSize) const
{
    return QImage(frameFileStr(frame, frameSize));
}

const QString VideoFrameSource_sV::frameFileStr(int number, FrameSize size) const
{
    QString dir;
    switch (size) {
    case FrameSize_Orig:
        dir = m_dirFramesOrig.absolutePath();
        break;
    case FrameSize_Small:
    default:
        dir = m_dirFramesSmall.absolutePath();
        break;
    }

    // ffmpeg numbering starts with 1, therefore add 1 to the frame number
    return QString("%1/frame%2.png").arg(dir).arg(number+1, 5, 10, QChar::fromAscii('0'));
}

void VideoFrameSource_sV::extractFramesFor(const FrameSize frameSize, QProcess *process)
{
    if (process != NULL) {
        qDebug() << "ffmpeg process is not NULL; terminating ...";
        process->waitForFinished(2000);
        delete process;
    }
    process = new QProcess();

    QStringList args;
    args << "-i" << m_inFile.fileName();
    args << "-f" << "image2";
    args << "-sameq";

    if (frameSize == FrameSize_Small) {
        int w = m_videoInfo->width;
        int h = m_videoInfo->height;
        // @todo adjust size
        while (w > 320) {
            w /= 2;
            h /= 2;
        }
        qDebug() << "Thumbnail frame size: " << w << "x" << h;
        args << "-s" << QString("%1x%2").arg(w).arg(h);
    }
    args << m_dirFramesSmall.absoluteFilePath("frame%5d.png");

    process->start("ffmpeg", args);
    qDebug() << process->readAllStandardOutput();
    qDebug() << process->readAllStandardError();
}

bool VideoFrameSource_sV::rebuildRequired(const FrameSize frameSize) const
{
    bool needsRebuild = false;

    QImage frame = frameAt(0, frameSize);
    needsRebuild |= frame.isNull();

    frame = frameAt(m_videoInfo->framesCount-1, frameSize);
    needsRebuild |= frame.isNull();

    return needsRebuild;
}

void VideoFrameSource_sV::slotExtractOrigFrames()
{
    emit signalNextTask("Extracting original-sized frames from the video file", m_videoInfo->framesCount);
    m_timer->start(100);
    if (rebuildRequired(FrameSize_Orig)) {
        if (m_ffmpeg != NULL) {
            qDebug() << "ffmpeg process is not NULL; terminating ...";
            m_ffmpeg->waitForFinished(2000);
            delete m_ffmpeg;
        }
        m_ffmpeg = new QProcess();

        bool b = true;
        b &= connect(m_ffmpeg, SIGNAL(finished(int)), this, SLOT(slotExtractSmallFrames()));
        Q_ASSERT(b);

        extractFramesFor(FrameSize_Orig, m_ffmpeg);
    } else {
        slotExtractSmallFrames();
    }
}

void VideoFrameSource_sV::slotExtractSmallFrames()
{
    emit signalNextTask("Extracting thumbnail-sized frames from the video file", m_videoInfo->framesCount);
    m_timer->start(100);
    if (rebuildRequired(FrameSize_Small)) {
        if (m_ffmpeg != NULL) {
            qDebug() << "ffmpeg process is not NULL; terminating ...";
            m_ffmpeg->waitForFinished(2000);
            delete m_ffmpeg;
        }
        m_ffmpeg = new QProcess();

        bool b = true;
        b &= connect(m_ffmpeg, SIGNAL(finished(int)), this, SLOT(slotInitializationFinished()));
        Q_ASSERT(b);

        extractFramesFor(FrameSize_Small, m_ffmpeg);
    } else {
        slotInitializationFinished();
    }
}

void VideoFrameSource_sV::slotInitializationFinished()
{
    m_timer->stop();
    emit signalAllTasksFinished();
}

void VideoFrameSource_sV::slotProgressUpdate()
{
    QRegExp regex(regexFrameNumber);
    QString s;
    if (m_ffmpeg != NULL) {
        s = QString(m_ffmpeg->readAllStandardError());
        if (regex.lastIndexIn(s) >= 0) {
            emit signalTaskProgress(regex.cap(1).toInt());
        }
    }
}
