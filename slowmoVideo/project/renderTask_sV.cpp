/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "renderTask_sV.h"
#include "abstractRenderTarget_sV.h"
#include "emptyFrameSource_sV.h"

#include <QImage>
#include <QMetaObject>
#include "project_sV.h"
#include "nodeList_sV.h"
#include "../lib/defs_sV.hpp"

RenderTask_sV::RenderTask_sV(Project_sV *project) :
    m_project(project),
    m_renderTarget(NULL),
    m_fps(24),
    m_initialized(false),
    m_stopRendering(false),
    m_prevTime(-1)
{
    m_timeStart = m_project->nodes()->startTime();
    m_timeEnd = m_project->nodes()->endTime();
    m_interpolationType = InterpolationType_Forward;

    setSize(FrameSize_Small);

    m_nextFrameTime = m_project->nodes()->startTime();
}

RenderTask_sV::~RenderTask_sV()
{
    if (m_renderTarget != NULL) { delete m_renderTarget; }
}

void RenderTask_sV::setRenderTarget(AbstractRenderTarget_sV *renderTarget)
{
    Q_ASSERT(renderTarget != NULL);

    if (m_renderTarget != NULL && m_renderTarget != renderTarget) {
        delete m_renderTarget;
    }
    m_renderTarget = renderTarget;
}

void RenderTask_sV::setTimeRange(float start, float end)
{
    Q_ASSERT(start <= end);
    Q_ASSERT(start >= m_project->nodes()->startTime());
    Q_ASSERT(end <= m_project->nodes()->endTime());

    m_timeStart = start;
    m_timeEnd = end;
}

void RenderTask_sV::setFPS(float fps)
{
    Q_ASSERT(fps > 0);
    m_fps = Fps_sV(fps);
}

void RenderTask_sV::setSize(FrameSize size)
{
    m_frameSize = size;
    m_resolution = const_cast<Project_sV*>(m_project)->frameSource()->frameAt(0, m_frameSize).size();
}

void RenderTask_sV::setInterpolationType(const InterpolationType interpolation)
{
    m_interpolationType = interpolation;
}

void RenderTask_sV::slotStopRendering()
{
    m_stopRendering = true;
}

void RenderTask_sV::slotContinueRendering()
{
    m_stopRendering = false;
    if (m_nextFrameTime < m_timeStart) {
        m_nextFrameTime = m_timeStart;
    }
    if (!m_initialized) {
        try {
            m_renderTarget->openRenderTarget();
            m_initialized = true;
        } catch (Error_sV &err) {
            m_stopRendering = true;
            emit signalRenderingAborted("Rendering aborted. " + err.message());
            return;
        }
    }
    qDebug() << "Continuing rendering at " << m_nextFrameTime;

    emit signalRenderingContinued();
    emit signalNewTask("Rendering slowmo ...", int(m_fps.fps() * (m_timeEnd-m_timeStart)));
    QMetaObject::invokeMethod(this, "slotRenderFrom", Qt::QueuedConnection, Q_ARG(qreal, m_nextFrameTime));
}

void RenderTask_sV::slotRenderFrom(qreal time)
{
    if (m_renderTarget == NULL) {
        m_stopRendering = true;
        emit signalRenderingAborted("No rendering target given! Aborting rendering.");
        return;
    }
    if (dynamic_cast<EmptyFrameSource_sV*>(const_cast<Project_sV*>(m_project)->frameSource()) != NULL) {
        m_stopRendering = true;
        emit signalRenderingAborted("Empty frame source, cannot be rendered.");
    }

    int outputFrame = (time - m_project->nodes()->startTime()) * m_fps.fps();
    if (!m_stopRendering) {

        if (time > m_timeEnd) {
            m_stopRendering = true;
            m_renderTarget->closeRenderTarget();
            emit signalRenderingFinished();

        } else {
            qreal srcTime = m_project->nodes()->sourceTime(time);

            qDebug() << "Rendering frame number " << outputFrame << " @" << time << " from source time " << srcTime;
            emit signalItemDesc(QString("Rendering frame %1 @ %2 s  from input position: %3 s (frame %4)")
                                .arg(outputFrame).arg(time).arg(srcTime).arg(outputFrame));
            try {
                QImage rendered = m_project->render(time, m_fps, m_interpolationType, m_frameSize);

                m_renderTarget->slotConsumeFrame(rendered, outputFrame);
                m_nextFrameTime = time + 1/m_fps.fps();

                emit signalTaskProgress(outputFrame);
                emit signalFrameRendered(time, outputFrame);
            } catch (FlowBuildingError &err) {
                m_stopRendering = true;
                emit signalRenderingAborted(err.message());
            } catch (InterpolationError &err) {
                emit signalItemDesc(err.message());
            }

            m_prevTime = srcTime;
        }

    } else {
        m_renderTarget->closeRenderTarget();
        emit signalRenderingStopped();
    }
    if (!m_stopRendering) {
        QMetaObject::invokeMethod(this, "slotRenderFrom", Qt::QueuedConnection, Q_ARG(qreal, m_nextFrameTime));
    }
}

