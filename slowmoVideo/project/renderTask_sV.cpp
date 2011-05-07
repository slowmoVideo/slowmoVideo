/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "renderTask_sV.h"

#include <QImage>
#include <QMetaObject>
#include "project_sV.h"
#include "nodelist_sV.h"

RenderTask_sV::RenderTask_sV(const Project_sV *project) :
    m_project(project),
    m_stopRendering(false)
{
    m_nextFrameTime = m_project->nodes()->startTime();
}

void RenderTask_sV::slotAbortRendering()
{
    m_stopRendering = true;
}

void RenderTask_sV::slotContinueRendering(qreal time)
{
    m_stopRendering = false;
    if (time >= 0) {
        m_nextFrameTime = time;
    }
    if (time < m_project->nodes()->startTime()) {
        m_nextFrameTime = m_project->nodes()->startTime();
    }
    qDebug() << "Continuing rendering at " << m_nextFrameTime;
    QMetaObject::invokeMethod(this, "slotRenderFrom", Qt::QueuedConnection, Q_ARG(qreal, m_nextFrameTime));
}

void RenderTask_sV::slotRenderFrom(qreal time)
{
    int frameNumber = (time - m_project->nodes()->startTime()) * m_project->fpsOut();
    if (!m_stopRendering) {

        if (time > m_project->nodes()->endTime()) {
            m_stopRendering = true;
            emit signalRenderingFinished();

        } else {
            qreal srcTime = m_project->nodes()->sourceTime(time);
            qDebug() << "Rendering frame number " << frameNumber << " @" << time << " from source time " << srcTime;
            QImage rendered = m_project->interpolateFrameAt(srcTime);
            rendered.save(m_project->renderedFileStr(frameNumber));
            m_nextFrameTime = time + 1/m_project->fpsOut();
            emit signalFrameRendered(time, frameNumber);
        }

    } else {
        emit signalRenderingAborted();
    }
    if (!m_stopRendering) {
        QMetaObject::invokeMethod(this, "slotRenderFrom", Qt::QueuedConnection, Q_ARG(qreal, m_nextFrameTime));
    }
}
