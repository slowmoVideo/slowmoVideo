/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "project_sV.h"

#include <cmath>

#include <QRegExp>

#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSignalMapper>
#include "../lib/opticalFlowBuilder_sV.h"
#include "../lib/opticalFlowBuilderGPUKLT_sV.h"
#include "../lib/interpolate_sV.h"

#define MIN_FRAME_DIST .001

QString Project_sV::defaultFramesDir("frames");
QString Project_sV::defaultThumbFramesDir("framesThumb");
QString Project_sV::defaultFlowDir("oFlow");
QRegExp Project_sV::regexFrameNumber("frame=\\s*(\\d+)");

Project_sV::Project_sV(QString filename, QString projectDir) :
    QObject(),
    m_canWriteFrames(false),
    m_flowComplete(false),
    m_inFile(filename),
    m_projDir(projectDir),
    m_ffmpegOrig(NULL),
    m_ffmpegSmall(NULL)
{
    m_videoInfo = getInfo(filename.toStdString().c_str());
    if (m_videoInfo.streamsCount <= 0) {
        qDebug() << "Video info is invalid.";
    }
    m_flow = new Flow_sV();

    // Create directories if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }
    m_framesDir = QDir(m_projDir.absolutePath() + "/" + defaultFramesDir);
    if (!m_framesDir.exists()) {
        m_framesDir.mkpath(".");
    }
    m_thumbFramesDir = QDir(m_projDir.absolutePath() + "/" + defaultThumbFramesDir);
    if (!m_thumbFramesDir.exists()) {
        m_thumbFramesDir.mkpath(".");
    }
    m_flowDir = QDir(m_projDir.absolutePath() + "/" + defaultFlowDir);
    if (!m_flowDir.exists()) {
        m_flowDir.mkpath(".");
    }
    m_canWriteFrames = validDirectories();


    m_timer = new QTimer(this);
    m_signalMapper = new QSignalMapper(this);

    bool b = true;
    b &= connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressUpdate()));
    b &= connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(slotExtractingFinished(int)));
    Q_ASSERT(b);
}

Project_sV::~Project_sV()
{
    delete m_signalMapper;
    delete m_timer;
    delete m_flow;
    if (m_ffmpegOrig != NULL) { delete m_ffmpegOrig; }
    if (m_ffmpegSmall != NULL) { delete m_ffmpegSmall; }
}

bool Project_sV::validDirectories() const
{
    bool valid = true;
    if (!m_projDir.exists()) {
        qDebug() << "Project directory could not be created.";
        valid = false;
    }
    if (!m_framesDir.exists()) {
        qDebug() << "Could not change to directory " << m_projDir.absolutePath() << "/" << defaultFramesDir;
        valid = false;
    } else {
        qDebug() << "Frame directory: " << m_framesDir.absolutePath();
    }
    if (!m_thumbFramesDir.exists()) {
        qDebug() << "Could not change to directory " << m_projDir.absolutePath() << "/" << defaultThumbFramesDir;
        valid = false;
    } else {
        qDebug() << "Frame thumb directory: " << m_thumbFramesDir.absolutePath();
    }
    if (!m_flowDir.exists()) {
        qDebug() << "Could not change to directory " << m_flowDir.absolutePath() << "/" << defaultFlowDir;
        valid = false;
    } else {
        qDebug() << "Flow directory: " << m_flowDir.absolutePath();
    }
    return valid;
}

const VideoInfoSV& Project_sV::videoInfo() const { return m_videoInfo; }
Flow_sV* Project_sV::flow() const { return m_flow; }

void Project_sV::render(qreal fps)
{
    // TODO slots: abort render, continue render
    //      signals: frame rendered, render finished, render aborted

}

void Project_sV::extractFrames(bool force)
{
    m_timer->start(100);
    m_processStatus.reset();
    if (rebuildRequired(FrameSize_Orig) || force) {
        extractFramesFor(FrameSize_Orig);
    } else {
        m_processStatus.origFinished = true;
        emit signalFramesExtracted(Project_sV::FrameSize_Orig);
    }
    if (rebuildRequired(FrameSize_Small) || force) {
        extractFramesFor(FrameSize_Small);
    } else {
        m_processStatus.smallFinished = true;
        emit signalFramesExtracted(Project_sV::FrameSize_Small);
    }
    if (m_processStatus.allFinished()) {
        m_timer->stop();
    }
}

bool Project_sV::extractFramesFor(const FrameSize frameSize)
{
    bool success = false;
    if (m_canWriteFrames) {
        QProcess *ffmpeg = new QProcess();

        QStringList args;
        args << "-i" << m_inFile.fileName();
        args << "-f" << "image2";
        args << "-sameq";

        switch (frameSize) {
        case FrameSize_Orig:
            {
                args << QString("%1/frame%5d.jpg").arg(m_framesDir.absolutePath());
                if (m_ffmpegOrig != NULL && m_ffmpegOrig->state() != QProcess::NotRunning) {
                    qDebug() << "Shutting down old ffmpeg process";
                    m_ffmpegOrig->waitForFinished(2000);
                    m_signalMapper->disconnect(m_ffmpegOrig, 0,0,0);
                    delete m_ffmpegOrig;
                    m_ffmpegOrig = NULL;
                }
                m_ffmpegOrig = ffmpeg;

                m_signalMapper->setMapping(m_ffmpegOrig, (int)FrameSize_Orig);
                bool b = true;
                b &= connect(m_ffmpegOrig, SIGNAL(finished(int)), m_signalMapper, SLOT(map()));
                Q_ASSERT(b);
            }
            break;
        case FrameSize_Small:
            {
                int w = m_videoInfo.width;
                int h = m_videoInfo.height;
                // TODO adjust size
                while (w > 320) {
                    w /= 2;
                    h /= 2;
                }
                qDebug() << "Thumbnail frame size: " << w << "x" << h;
                args << "-s" << QString("%1x%2").arg(w).arg(h);
                args << QString("%1/frame%5d.jpg").arg(m_thumbFramesDir.absolutePath());
                if (m_ffmpegSmall != NULL && m_ffmpegSmall->state() != QProcess::NotRunning) {
                    qDebug() << "Shutting down old ffmpeg process";
                    m_ffmpegSmall->waitForFinished(2000);
                    m_signalMapper->disconnect(m_ffmpegSmall, 0,0,0);
                    delete m_ffmpegSmall;
                    m_ffmpegSmall = NULL;
                }
                m_ffmpegSmall = ffmpeg;

                m_signalMapper->setMapping(m_ffmpegSmall, (int)FrameSize_Small);
                bool b = true;
                b &= connect(m_ffmpegSmall, SIGNAL(finished(int)), m_signalMapper, SLOT(map()));
                Q_ASSERT(b);

            }
            break;
        }


        ffmpeg->start("ffmpeg", args);
        qDebug() << ffmpeg->readAllStandardOutput();
        qDebug() << ffmpeg->readAllStandardError();

        if (ffmpeg->exitCode() == 0) {
            success = true;
        }

    }
    return success;
}

bool Project_sV::rebuildRequired(const FrameSize frameSize) const
{
    bool needsRebuild = false;

    QImage frame = frameAt(0, frameSize);
    needsRebuild |= frame.isNull();

    frame = frameAt(m_videoInfo.framesCount-1, frameSize);
    needsRebuild |= frame.isNull();

    return needsRebuild;
}

QImage Project_sV::frameAt(const uint frame, const FrameSize frameSize) const
{
    QString filename;
    switch (frameSize) {
    case FrameSize_Orig:
        filename = frameFileStr(frame);
        break;
    case FrameSize_Small:
        filename = thumbFileStr(frame);
        break;
    }

    return QImage(filename);
}

QImage Project_sV::interpolateFrameAt(float time) const
{
    float framePos = timeToFrame(time);
    if (framePos > m_videoInfo.framesCount) {
        qDebug() << "Requested frame " << framePos << ": Not within valid range. (" << m_videoInfo.framesCount << " frames)";
        Q_ASSERT(false);
    }
    if (framePos-floor(framePos) < MIN_FRAME_DIST) {
        QImage left(frameFileStr(floor(framePos)));
        QImage right(frameFileStr(floor(framePos)+1));
        QImage out(m_videoInfo.width, m_videoInfo.height, QImage::Format_RGB888);
        Interpolate_sV::forwardFlow(left, right, framePos-floor(framePos), out);
        return out;
    } else {
        return QImage(frameFileStr(floor(framePos)));
    }
}

QImage Project_sV::requestFlow(int leftFrame, FlowDirection direction, bool forceRebuild) const
{
    Q_ASSERT(leftFrame < m_videoInfo.framesCount-1);
    const QString outFile = flowFileStr(leftFrame, direction);
    if (!QFile(outFile).exists() || forceRebuild) {
        qDebug() << "Building flow for left frame " << leftFrame << " in direction " << direction;
        const QString left(thumbFileStr(leftFrame));
        const QString right(thumbFileStr(leftFrame+1));
        m_flow->buildFlowImage(left, right,
                               outFile, direction);
    } else {
        qDebug() << "Re-using existing flow image for left frame " << leftFrame << " in direction " << direction;
    }
    return QImage(outFile);
}

inline
float Project_sV::timeToFrame(float time) const
{
    Q_ASSERT(time >= 0);
    return time * m_videoInfo.frameRateNum / m_videoInfo.frameRateDen;
}

const QString Project_sV::frameFileStr(int number) const
{
    // ffmpeg numbering starts with 1, therefore add 1 to the frame number
    return QString(m_framesDir.absoluteFilePath("frame%1.jpg").arg(number+1, 5, 10, QChar::fromAscii('0')));
}
const QString Project_sV::thumbFileStr(int number) const
{
    return QString(m_thumbFramesDir.absoluteFilePath("frame%1.jpg").arg(number+1, 5, 10, QChar::fromAscii('0')));
}
const QString Project_sV::flowFileStr(int leftFrame, FlowDirection direction) const
{
    switch (direction) {
    case FlowDirection_Forward:
        return QString(m_flowDir.absoluteFilePath("forward%1.jpg").arg(leftFrame+1, 5, 10, QChar::fromAscii('0')));
    case FlowDirection_Backward:
        return QString(m_flowDir.absoluteFilePath("backward%1.jpg").arg(leftFrame+2, 5, 10, QChar::fromAscii('0')));
    }
}

void Project_sV::slotExtractingFinished(int fs)
{
    Project_sV::FrameSize frameSize = (FrameSize) fs;
    qDebug() << "Finished: " << frameSize;

    switch(frameSize) {
    case FrameSize_Orig:
        emit signalFramesExtracted(Project_sV::FrameSize_Orig);
        m_processStatus.origFinished = true;
        break;
    case FrameSize_Small:
        emit signalFramesExtracted(Project_sV::FrameSize_Small);
        m_processStatus.smallFinished = true;
        break;
    }
    if (m_processStatus.allFinished()) {
        qDebug() << "All finished, stopping timer.";
        m_timer->stop();
    }
}

void Project_sV::slotProgressUpdate()
{
    QRegExp regex(regexFrameNumber);
    QString s;
    if (m_ffmpegOrig != NULL) {
        s = QString(m_ffmpegOrig->readAllStandardError());
        if (regex.indexIn(s) >= 0) {
            emit signalProgressUpdated(Project_sV::FrameSize_Orig, (100*regex.cap(1).toInt())/m_videoInfo.framesCount);
        }
    }
    if (m_ffmpegSmall != NULL) {
        s = QString(m_ffmpegSmall->readAllStandardError());
        if (regex.indexIn(s) >= 0) {
            emit signalProgressUpdated(Project_sV::FrameSize_Small, (100*regex.cap(1).toInt())/m_videoInfo.framesCount);
        }
    }
}

void Project_sV::slotFlowCompleted()
{
    m_flowComplete = true;
}

QDebug operator<<(QDebug qd, const Project_sV::FrameSize &frameSize)
{
    switch(frameSize) {
    case Project_sV::FrameSize_Orig:
        qd << "Original frame size";
        break;
    case Project_sV::FrameSize_Small:
        qd << "Small frame size";
        break;
    }
    return qd;
}
