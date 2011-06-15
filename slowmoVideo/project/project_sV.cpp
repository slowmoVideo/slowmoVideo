/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "videoFrameSource_sV.h"
#include "../lib/opticalFlowBuilder_sV.h"
#include "../lib/opticalFlowBuilderGPUKLT_sV.h"
#include "../lib/interpolate_sV.h"
#include "../lib/flowRW_sV.h"
#include "../lib/flowField_sV.h"

#include <cmath>

#include <QRegExp>

#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSignalMapper>

#include "renderTask_sV.h"
#include "nodelist_sV.h"
#include "flow_sV.h"

#define MIN_FRAME_DIST .001

QRegExp Project_sV::regexFrameNumber("frame=\\s*(\\d+)");

Project_sV::Project_sV()
{
    init();
}

Project_sV::Project_sV(QString filename, QString projectDir)
{
    init();
    loadFile(filename, projectDir);
}

void Project_sV::init()
{
    m_canWriteFrames = false;
    m_flowComplete = false;
//    m_videoInfo = NULL;
//    m_ffmpegOrig = NULL;
//    m_ffmpegSmall = NULL;
    m_fps = 24;
    m_renderFrameSize = FrameSize_Small;

    m_frameSource = new VideoFrameSource_sV(this, "/tmp/noexist.avi");

//    m_videoInfo = new VideoInfoSV();
    m_flow = new Flow_sV();
    m_tags = new QList<Tag_sV>();
    m_nodes = new NodeList_sV();
    m_renderTask = new RenderTask_sV(this);

    m_timer = new QTimer(this);
    m_signalMapper = new QSignalMapper(this);

    bool b = true;
    b &= connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressUpdate()));
    b &= connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(slotExtractingFinished(int)));
    Q_ASSERT(b);

    qDebug() << "Tag list address: " << m_tags;
}

Project_sV::~Project_sV()
{
    delete m_signalMapper;
    delete m_timer;
    delete m_flow;
    delete m_tags;
    delete m_nodes;
    delete m_renderTask;
//    delete m_videoInfo;
//    if (m_ffmpegOrig != NULL) { delete m_ffmpegOrig; }
//    if (m_ffmpegSmall != NULL) { delete m_ffmpegSmall; }
}

float Project_sV::length() const
{
    if (m_nodes->size() > 0) {
        return m_nodes->at(m_nodes->size()-1).xUnmoved();
    } else {
        return 0;
    }
}

void Project_sV::loadFile(QString filename, QString projectDir)
{
    // TODO
    // Set fps number
//    m_inFile.setFileName(filename);
    m_projDir = projectDir;

//    *m_videoInfo = getInfo(filename.toStdString().c_str());
//    if (m_videoInfo->streamsCount <= 0) {
//        qDebug() << "Video info is invalid: " << filename;
//    } else {
//        m_fps = m_videoInfo->frameRateNum/(float)m_videoInfo->frameRateDen;
//    }

    // Create directories if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }

//    createDirectories(FrameSize_Orig);
//    createDirectories(FrameSize_Small);
//    m_canWriteFrames = validDirectories();
    // TODO delete ^
}

const QDir Project_sV::getDirectory(const QString &name, bool createIfNotExists) const
{
    QDir dir(m_projDir.absolutePath() + "/" + name);
    if (createIfNotExists && !dir.exists()) {
        dir.mkpath(".");
    }
    return dir;
}

bool Project_sV::validDirectories() const
{
    bool valid = true;
    if (!m_projDir.exists()) {
        qDebug() << "Project directory could not be created.";
        valid = false;
    }
    QList<QDir> dirList;
    dirList << QDir(framesDirStr(FrameSize_Orig)) << QDir(flowDirStr(FrameSize_Orig)) << QDir(renderDirStr(FrameSize_Orig))
               << QDir(framesDirStr(FrameSize_Small)) << QDir(flowDirStr(FrameSize_Small)) << QDir(renderDirStr(FrameSize_Small));
    for (int i = 0; i < dirList.size(); i++) {
        if (!dirList.at(i).exists()) {
            valid = false;
            qDebug() << "Directory " << dirList.at(i).absolutePath() << " does not exist.";
            break;
        }
    }
    return valid;
}/*

void Project_sV::extractFrames(bool force)
{
    m_timer->start(100);
    m_processStatus.reset();
    if (rebuildRequired(FrameSize_Orig) || force) {
        extractFramesFor(FrameSize_Orig);
    } else {
        m_processStatus.origFinished = true;
        emit signalFramesExtracted(FrameSize_Orig);
    }
    if (rebuildRequired(FrameSize_Small) || force) {
        extractFramesFor(FrameSize_Small);
    } else {
        m_processStatus.smallFinished = true;
        emit signalFramesExtracted(FrameSize_Small);
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
                args << QString("%1/frame%5d.jpg").arg(framesDirStr(frameSize));
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
                int w = m_videoInfo->width;
                int h = m_videoInfo->height;
                // @todo adjust size
                while (w > 320) {
                    w /= 2;
                    h /= 2;
                }
                qDebug() << "Thumbnail frame size: " << w << "x" << h;
                args << "-s" << QString("%1x%2").arg(w).arg(h);
                args << QString("%1/frame%5d.png").arg(framesDirStr(frameSize));
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
}*/

//bool Project_sV::rebuildRequired(const FrameSize frameSize) const
//{
//    bool needsRebuild = false;

//    QImage frame = frameAt(0, frameSize);
//    needsRebuild |= frame.isNull();

//    frame = frameAt(m_videoInfo->framesCount-1, frameSize);
//    needsRebuild |= frame.isNull();

//    return needsRebuild;
//}

//QImage Project_sV::frameAt(const uint frame, const FrameSize frameSize) const
//{
//    return QImage(frameFileStr(frame, frameSize));
//}

/**
  @todo frame size
  */
QImage Project_sV::interpolateFrameAt(float time, const FrameSize frameSize) const
{
    float framePos = timeToFrame(time);
    if (framePos > m_frameSource->framesCount()) {
        qDebug() << "Requested frame " << framePos << ": Not within valid range. (" << m_frameSource->framesCount() << " frames)";
        Q_ASSERT(false);
    } else {
        qDebug() << "Source frame @" << time << " is " << framePos;
    }
    if (framePos-floor(framePos) > MIN_FRAME_DIST) {

        QImage left(frameFileStr(floor(framePos), frameSize));
        QImage right(frameFileStr(floor(framePos)+1, frameSize));
        QImage out(left.size(), QImage::Format_RGB888);

        FlowField_sV *forwardFlow = requestFlow(floor(framePos), FlowDirection_Forward, frameSize);
        FlowField_sV *backwardFlow = requestFlow(floor(framePos), FlowDirection_Backward, frameSize);

        Q_ASSERT(forwardFlow != NULL);
        Q_ASSERT(backwardFlow != NULL);

        if (forwardFlow == NULL || backwardFlow == NULL) {
            qDebug() << "No flow received!";
            Q_ASSERT(false);
        }

        Interpolate_sV::twowayFlow(left, right, forwardFlow, backwardFlow, framePos-floor(framePos), out);

        return out;
    } else {
        qDebug() << "No interpolation necessary.";
        return QImage(frameFileStr(floor(framePos), frameSize));
    }
}

FlowField_sV* Project_sV::requestFlow(int leftFrame, FlowDirection direction, const FrameSize frameSize, bool forceRebuild) const
{
    Q_ASSERT(leftFrame < m_frameSource->framesCount()-1);
    const QString outFile = flowFileStr(leftFrame, direction, frameSize);
    if (!QFile(outFile).exists() || forceRebuild) {
        qDebug() << "Building flow for left frame " << leftFrame << " in direction " << direction << "; Size: " << frameSize;
        const QString left = frameFileStr(leftFrame, frameSize);
        const QString right = frameFileStr(leftFrame+1, frameSize);
        m_flow->buildFlowImage(left, right,
                               outFile, direction);
    } else {
        qDebug() << "Re-using existing flow image for left frame " << leftFrame << " in direction " << direction << ": " << outFile;
    }
    return FlowRW_sV::load(outFile.toStdString());
}

inline
float Project_sV::timeToFrame(float time) const
{
    Q_ASSERT(time >= 0);
    return time * m_frameSource->fps();
}


const QString Project_sV::framesDirStr(FrameSize frameSize) const
{
    return m_projDir.absolutePath() + "/frames" + enumStr(frameSize);
}
const QString Project_sV::flowDirStr(FrameSize frameSize) const
{
    return m_projDir.absolutePath() + "/oFlow" + enumStr(frameSize);
}
const QString Project_sV::renderDirStr(FrameSize frameSize) const
{
    return m_projDir.absolutePath() + "/rendered" + enumStr(frameSize);
}

void Project_sV::createDirectories(FrameSize frameSize) const
{
    QList<QDir> dirList;
    dirList << QDir(framesDirStr(frameSize)) << QDir(flowDirStr(frameSize)) << QDir(renderDirStr(frameSize));
    for (int i = 0; i < dirList.size(); i++) {
        if (!dirList.at(i).exists()) {
            dirList.at(i).mkpath(".");
            qDebug() << "Created directory: " << dirList.at(i).absolutePath();
        }
    }
}

const QString Project_sV::frameFileStr(int number, FrameSize size) const
{
    // ffmpeg numbering starts with 1, therefore add 1 to the frame number
    return QString("%1/frame%2.png").arg(framesDirStr(size)).arg(number+1, 5, 10, QChar::fromAscii('0'));
}
const QString Project_sV::flowFileStr(int leftFrame, FlowDirection direction, FrameSize size) const
{
    switch (direction) {
    case FlowDirection_Forward:
        return QString("%1/forward%2.sVflow").arg(flowDirStr(size)).arg(leftFrame+1, 5, 10, QChar::fromAscii('0'));
    case FlowDirection_Backward:
    default:
        return QString("%1/backward%2.sVflow").arg(flowDirStr(size)).arg(leftFrame+2, 5, 10, QChar::fromAscii('0'));
    }
}
const QString Project_sV::renderedFileStr(int number, FrameSize size) const
{
    return QString("%1/rendered%2.jpg").arg(renderDirStr(size)).arg(number+1, 5, 10, QChar::fromAscii('0'));
}

void Project_sV::slotExtractingFinished(int fs)
{
    FrameSize frameSize = (FrameSize) fs;
    qDebug() << "Finished: " << frameSize;

    switch(frameSize) {
    case FrameSize_Orig:
        emit signalFramesExtracted(FrameSize_Orig);
        m_processStatus.origFinished = true;
        break;
    case FrameSize_Small:
        emit signalFramesExtracted(FrameSize_Small);
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
    // TODO
//    if (m_ffmpegOrig != NULL) {
//        s = QString(m_ffmpegOrig->readAllStandardError());
//        if (regex.indexIn(s) >= 0) {
//            emit signalProgressUpdated(FrameSize_Orig, (100*regex.cap(1).toInt())/m_frameSource->framesCount());
//        }
//    }
//    if (m_ffmpegSmall != NULL) {
//        s = QString(m_ffmpegSmall->readAllStandardError());
//        if (regex.indexIn(s) >= 0) {
//            emit signalProgressUpdated(FrameSize_Small, (100*regex.cap(1).toInt())/m_frameSource->framesCount());
//        }
//    }
}

void Project_sV::slotFlowCompleted()
{
    m_flowComplete = true;
}

void Project_sV::slotSetFps(float fps)
{
    Q_ASSERT(fps > 0);
    m_fps = fps;
}
void Project_sV::slotSetRenderFrameSize(const FrameSize size)
{
    qDebug() << "Render frame size is now " << size;
    m_renderFrameSize = size;
}
