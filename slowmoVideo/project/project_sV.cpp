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

#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "renderTask_sV.h"
#include "nodelist_sV.h"
#include "flow_sV.h"

#define MIN_FRAME_DIST .001

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
    m_fps = 24;
    m_renderFrameSize = FrameSize_Small;

    m_frameSource = new VideoFrameSource_sV(this, "/tmp/noexist.avi");

    m_flow = new Flow_sV();
    m_tags = new QList<Tag_sV>();
    m_nodes = new NodeList_sV();
    m_renderTask = new RenderTask_sV(this);

    qDebug() << "Tag list address: " << m_tags;
}

Project_sV::~Project_sV()
{
    delete m_frameSource;
    delete m_flow;
    delete m_tags;
    delete m_nodes;
    delete m_renderTask;
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
    m_projDir = projectDir;

    // Create directory if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }
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
}

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
