/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "project_sV.h"
#include "videoFrameSource_sV.h"
#include "emptyFrameSource_sV.h"
#include "v3dFlowSource_sV.h"
#include "../lib/shutter_sV.h"
#include "../lib/interpolate_sV.h"
#include "../lib/flowRW_sV.h"
#include "../lib/flowField_sV.h"

#include <cmath>

#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "renderTask_sV.h"
#include "nodelist_sV.h"

#define MIN_FRAME_DIST .001

Project_sV::Project_sV() :
    m_projDir(QDir::temp())
{
    init();
}

Project_sV::Project_sV(QString projectDir) :
    m_projDir(projectDir)
{
    init();

    // Create directory if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }
}

void Project_sV::init()
{
    m_frameSource = new EmptyFrameSource_sV(this);
    m_flowSource = new V3dFlowSource_sV(this);

    m_tags = new QList<Tag_sV>();
    m_nodes = new NodeList_sV();
    m_renderTask = NULL;
}

Project_sV::~Project_sV()
{
    delete m_frameSource;
    delete m_flowSource;
    delete m_tags;
    delete m_nodes;
    delete m_renderTask;
}

void Project_sV::setProjectDir(QString projectDir)
{
    m_projDir = projectDir;
    // Create directory if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }
    m_frameSource->slotUpdateProjectDir();
    m_flowSource->slotUpdateProjectDir();
}

void Project_sV::readSettings(const QSettings &settings)
{
    if (dynamic_cast<V3dFlowSource_sV *>(m_flowSource) != NULL) {
        V3dFlowSource_sV *flowSource = dynamic_cast<V3dFlowSource_sV *>(m_flowSource);
        flowSource->setLambda(settings.value("settings/v3dFlowBuilder/lambda", "5.0").toDouble());
        qDebug() << "Lambda set to " << settings.value("settings/v3dFlowBuilder/lambda", "5.0").toDouble();
    }
}

void Project_sV::loadFrameSource(AbstractFrameSource_sV *frameSource)
{
    if (m_frameSource != NULL) {
        delete m_frameSource;
    }
    if (frameSource == NULL) {
        m_frameSource = new EmptyFrameSource_sV(this);
    } else {
        m_frameSource = frameSource;
    }
}

void Project_sV::replaceRenderTask(RenderTask_sV *task)
{
    if (m_renderTask != NULL) {
        m_renderTask->slotStopRendering();
        m_renderTask->deleteLater();
        m_renderTask = NULL;
    }
    m_renderTask = task;
}

const QDir Project_sV::getDirectory(const QString &name, bool createIfNotExists) const
{
    QDir dir(m_projDir.absolutePath() + "/" + name);
    if (createIfNotExists && !dir.exists()) {
        dir.mkpath(".");
    }
    return dir;
}

QImage Project_sV::interpolateFrameAt(float time, const FrameSize frameSize, float previousTime) const throw(FlowBuildingError)
{
    float framePos = timeToFrame(time);
    float prevFramePos = -1;
    if (framePos > m_frameSource->framesCount()) {
        qDebug() << "Requested frame " << framePos << ": Not within valid range. (" << m_frameSource->framesCount() << " frames)";
        Q_ASSERT(false);
        // TODO throw error
    } else {
        qDebug() << "Source frame @" << time << " is " << framePos;
    }
    if (previousTime >= 0) {
        prevFramePos = timeToFrame(previousTime);
    }

    if (prevFramePos >= 0 && fabs(framePos-prevFramePos) > 1) {
        QStringList frames;
        int left = std::min(floor(prevFramePos), floor(framePos));
        int right = std::max(ceil(prevFramePos), ceil(framePos));
        for (int f = left; f <= right; f++) {
            frames << m_frameSource->framePath(f, frameSize);
        }
        qDebug() << "Simulating shutter between frames " << floor(prevFramePos) << " and " << ceil(framePos);
        return Shutter_sV::combine(frames);
    } else if (framePos-floor(framePos) > MIN_FRAME_DIST) {

        QImage left = m_frameSource->frameAt(floor(framePos), frameSize);
        QImage right = m_frameSource->frameAt(floor(framePos)+1, frameSize);
        QImage out(left.size(), QImage::Format_RGB888);

        FlowField_sV *forwardFlow = requestFlow(floor(framePos), floor(framePos)+1, frameSize);
        FlowField_sV *backwardFlow = requestFlow(floor(framePos)+1, floor(framePos), frameSize);

        Q_ASSERT(forwardFlow != NULL);
        Q_ASSERT(backwardFlow != NULL);

        if (forwardFlow == NULL || backwardFlow == NULL) {
            qDebug() << "No flow received!";
            Q_ASSERT(false);
        }

        Interpolate_sV::twowayFlow(left, right, forwardFlow, backwardFlow, framePos-floor(framePos), out);

        delete forwardFlow;
        delete backwardFlow;
        return out;
    } else {
        qDebug() << "No interpolation necessary.";
        return m_frameSource->frameAt(floor(framePos), frameSize);
    }
}

FlowField_sV* Project_sV::requestFlow(int leftFrame, int rightFrame, const FrameSize frameSize) const throw(FlowBuildingError)
{
    Q_ASSERT(leftFrame < m_frameSource->framesCount());
    Q_ASSERT(rightFrame < m_frameSource->framesCount());
    return m_flowSource->buildFlow(leftFrame, rightFrame, frameSize);
}

inline
float Project_sV::timeToFrame(float time) const
{
    Q_ASSERT(time >= 0);
    return time * m_frameSource->fps();
}

