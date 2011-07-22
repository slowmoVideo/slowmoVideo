/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "project_sV.h"
#include "projectPreferences_sV.h"
#include "videoFrameSource_sV.h"
#include "emptyFrameSource_sV.h"
#include "v3dFlowSource_sV.h"
#include "interpolator_sV.h"
#include "motionBlur_sV.h"
#include "shutterFunction_sV.h"
#include "shutterFunctionList_sV.h"
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
    m_preferences = new ProjectPreferences_sV();
    m_frameSource = new EmptyFrameSource_sV(this);
    m_flowSource = new V3dFlowSource_sV(this);
    m_motionBlur = new MotionBlur_sV(this, 3); ///< \todo motion blur samples as argument

    m_tags = new QList<Tag_sV>();
    m_nodes = new NodeList_sV();
    m_shutterFunctions = new ShutterFunctionList_sV();
    m_renderTask = NULL;
}

Project_sV::~Project_sV()
{
    delete m_preferences;
    delete m_frameSource;
    delete m_flowSource;
    delete m_motionBlur;
    delete m_tags;
    delete m_nodes;
    delete m_renderTask;
    delete m_shutterFunctions;
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
    m_motionBlur->slotUpdateProjectDir();
}

void Project_sV::setProjectFilename(QString filename)
{
    m_projectFilename = filename;
}
QString Project_sV::projectFilename() const {
    return m_projectFilename;
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
    m_nodes->setMaxY(m_frameSource->maxTime());
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

QImage Project_sV::render(float outTime, Fps_sV fps, InterpolationType interpolation, FrameSize size)
{
    if (outTime < 0 || outTime > m_nodes->endTime()) {
        qDebug() << "Output time out of bounds";
        Q_ASSERT(false);
    }

    float sourceTime = m_nodes->sourceTime(outTime);
    if (sourceTime < 0) {
        sourceTime = 0;
    }
    if (sourceTime > m_frameSource->maxTime()) {
        sourceTime = m_frameSource->maxTime();
    }

    float sourceFrame = sourceTimeToFrame(sourceTime);

    int leftIndex = m_nodes->find(outTime);
    if (leftIndex < 0) {
        qDebug() << "left node is not here!";
        Q_ASSERT(false);
    }
    if (leftIndex == m_nodes->size()-1) {
        // outTime is at the very end of the node.
        // Take next to last node to still have a right node.
        leftIndex--;
    }

    const Node_sV *leftNode = &(*m_nodes)[leftIndex];
    const Node_sV *rightNode = &(*m_nodes)[leftIndex+1];

    ShutterFunction_sV *shutterFunction = m_shutterFunctions->function(leftNode->shutterFunctionID());

    if (shutterFunction != NULL) {
        float shutter = shutterFunction->evaluate(
                    (sourceFrame-floor(sourceFrame))/fps.fps(),
                    fps.fps(),
                    rightNode->y()-leftNode->y(),
                    outTime-m_nodes->startTime()
                    );
        return m_motionBlur->blur(sourceFrame, sourceFrame+shutter, fabs((rightNode->y()-leftNode->y())/fps.fps()), size);
    } else {
        return Interpolator_sV::interpolate(this, sourceFrame, interpolation, size);
    }
}

FlowField_sV* Project_sV::requestFlow(int leftFrame, int rightFrame, const FrameSize frameSize) const throw(FlowBuildingError)
{
    Q_ASSERT(leftFrame < m_frameSource->framesCount());
    Q_ASSERT(rightFrame < m_frameSource->framesCount());
    return m_flowSource->buildFlow(leftFrame, rightFrame, frameSize);
}

inline
float Project_sV::sourceTimeToFrame(float time) const
{
    Q_ASSERT(time >= 0);
    return time * m_frameSource->fps();
}

QList<NodeList_sV::PointerWithDistance> Project_sV::objectsNear(QPointF pos, qreal tmaxdist) const
{
    QList<NodeList_sV::PointerWithDistance> list = m_nodes->objectsNear(pos, tmaxdist);

    qreal dist;
    for (int i = 0; i < m_tags->size(); i++) {
        if (fabs(m_tags->at(i).axis() == TagAxis_Source)) {
            dist = fabs(pos.y() - m_tags->at(i).time());
        } else {
            dist = fabs(pos.x() - m_tags->at(i).time());
        }
        if (dist <= tmaxdist) {
            list << NodeList_sV::PointerWithDistance(&m_tags->at(i), dist, NodeList_sV::PointerWithDistance::Tag);
        }
    }

    qSort(list);

    return list;
}

