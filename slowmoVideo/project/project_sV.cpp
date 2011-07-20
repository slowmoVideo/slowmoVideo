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
#include "motionBlur_sV.h"
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
    m_motionBlur = new MotionBlur_sV(this);

    m_tags = new QList<Tag_sV>();
    m_nodes = new NodeList_sV();
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

QImage Project_sV::interpolateFrameAt(float frame, const FrameSize frameSize, const InterpolationType interpolation) const
throw(FlowBuildingError, InterpolationError)
{
    if (frame > m_frameSource->framesCount()) {
        throw InterpolationError(QString("Requested frame %1: Not within valid range. (%2 frames)")
                                 .arg(frame).arg(m_frameSource->framesCount()));
    }
    if (frame-floor(frame) > MIN_FRAME_DIST) {

        QImage left = m_frameSource->frameAt(floor(frame), frameSize);
        QImage right = m_frameSource->frameAt(floor(frame)+1, frameSize);
        QImage out(left.size(), QImage::Format_ARGB32);

        /// Position between two frames, on [0 1]
        const float pos = frame-floor(frame);

        if (interpolation == InterpolationType_Twoway) {
            FlowField_sV *forwardFlow = requestFlow(floor(frame), floor(frame)+1, frameSize);
            FlowField_sV *backwardFlow = requestFlow(floor(frame)+1, floor(frame), frameSize);

            Q_ASSERT(forwardFlow != NULL);
            Q_ASSERT(backwardFlow != NULL);

            if (forwardFlow == NULL || backwardFlow == NULL) {
                qDebug() << "No flow received!";
                Q_ASSERT(false);
            }

            Interpolate_sV::twowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
            delete forwardFlow;
            delete backwardFlow;

        } else if (interpolation == InterpolationType_TwowayNew) {
            FlowField_sV *forwardFlow = requestFlow(floor(frame), floor(frame)+1, frameSize);
            FlowField_sV *backwardFlow = requestFlow(floor(frame)+1, floor(frame), frameSize);

            Q_ASSERT(forwardFlow != NULL);
            Q_ASSERT(backwardFlow != NULL);

            if (forwardFlow == NULL || backwardFlow == NULL) {
                qDebug() << "No flow received!";
                Q_ASSERT(false);
            }

            Interpolate_sV::newTwowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
            delete forwardFlow;
            delete backwardFlow;

        } else if (interpolation == InterpolationType_Forward) {
            FlowField_sV *forwardFlow = requestFlow(floor(frame), floor(frame)+1, frameSize);

            Q_ASSERT(forwardFlow != NULL);

            if (forwardFlow == NULL) {
                qDebug() << "No flow received!";
                Q_ASSERT(false);
            }

            Interpolate_sV::forwardFlow(left, forwardFlow, pos, out);
            delete forwardFlow;

        } else if (interpolation == InterpolationType_ForwardNew) {
            FlowField_sV *forwardFlow = requestFlow(floor(frame), floor(frame)+1, frameSize);

            Q_ASSERT(forwardFlow != NULL);

            if (forwardFlow == NULL) {
                qDebug() << "No flow received!";
                Q_ASSERT(false);
            }

            Interpolate_sV::newForwardFlow(left, forwardFlow, pos, out);
            delete forwardFlow;

        } else if (interpolation == InterpolationType_Bezier) {
            FlowField_sV *currNext = requestFlow(floor(frame)+2, floor(frame)+1, frameSize); // Allowed to be NULL
            FlowField_sV *currPrev = requestFlow(floor(frame)+0, floor(frame)+1, frameSize);

            Q_ASSERT(currPrev != NULL);

            Interpolate_sV::bezierFlow(left, right, currPrev, currNext, pos, out);

            delete currNext;
            delete currPrev;

        } else {
            qDebug() << "Unsupported interpolation type!";
            Q_ASSERT(false);
        }
        return out;
    } else {
        qDebug() << "No interpolation necessary.";
        return m_frameSource->frameAt(floor(frame), frameSize);
    }
}

QImage Project_sV::interpolateFrameAtTime(float time, const FrameSize frameSize, const InterpolationType interpolation,
                                      float previousTime) const throw(FlowBuildingError, InterpolationError)
{
    float framePos = timeToFrame(time);
    float prevFramePos = -1;
    if (framePos > m_frameSource->framesCount()) {
        throw InterpolationError(QString("Requested frame %1: Not within valid range. (%2 frames)")
                                 .arg(framePos).arg(m_frameSource->framesCount()));
    } else {
        qDebug() << "Source frame @" << time << " is " << framePos;
    }
    if (previousTime >= 0) {
        prevFramePos = timeToFrame(previousTime);
    }

    if (prevFramePos >= 0 && fabs(framePos-prevFramePos) > 1.2) {
//        QStringList frames;
//        int left = std::min(floor(prevFramePos), floor(framePos));
//        int right = std::max(ceil(prevFramePos), ceil(framePos));
//        for (int f = left; f <= right; f++) {
//            frames << m_frameSource->framePath(f, frameSize);
//        }
//        qDebug() << "Simulating shutter between frames " << floor(prevFramePos) << " and " << ceil(framePos);
//        return Shutter_sV::combine(frames);
        qDebug() << "Simulating NEW shutter between frames " << floor(prevFramePos) << " and " << ceil(framePos);
        return m_motionBlur->blur(prevFramePos, framePos, 4, frameSize);

    } else {
        return interpolateFrameAt(framePos, frameSize, interpolation);
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

