/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef RENDERTASK_SV_H
#define RENDERTASK_SV_H

#include <QtCore/QObject>
#include <QtCore/QTime>

#include "../lib/defs_sV.hpp"

class Project_sV;
class AbstractRenderTarget_sV;

/**
  \brief Renders a project when started.
  */
class RenderTask_sV : public QObject
{
    Q_OBJECT
public:
    RenderTask_sV(Project_sV *project);
    ~RenderTask_sV();

    /**
      \fn setRenderTarget()
      Manages the \c renderTarget pointer (includes destruction).
      */
    /**
      \fn setTimeRange()
      Sets the time range (in seconds) for rendering. By default, the whole project is rendered.
      \todo Accept other formats: f:123 for frames, t:Tag for tags
      */
    /**
      \fn setFPS()
      Sets the number of frames per second for rendering.
      */
    /**
      \fn setSize()
      Sets the size to use for rendering.
      */
    void setRenderTarget(AbstractRenderTarget_sV *renderTarget);
    void setTimeRange(qreal start, qreal end);
    void setFPS(const Fps_sV fps);
    void setSize(FrameSize size);
    void setInterpolationType(const InterpolationType interpolation);

    void setQtConnectionType(Qt::ConnectionType type);

    /// Rendered frames per second
    Fps_sV fps() { return m_fps; }
    /// Output frame resolution
    QSize resolution() { return m_resolution; }


public slots:
    void slotContinueRendering();
    void slotStopRendering();
    // reset?

signals:
    void signalNewTask(QString desc, int taskSize);
    void signalItemDesc(QString desc);
    void signalTaskProgress(int value);
    void signalRenderingContinued();
    void signalRenderingStopped(QString renderTime);
    void signalRenderingFinished(QString renderTime);
    void signalRenderingAborted(QString reason);

    void signalFrameRendered(qreal time, int frameNumber);

private:
    Project_sV *m_project;
    AbstractRenderTarget_sV *m_renderTarget;

    qreal m_timeStart;
    qreal m_timeEnd;

    QTime m_stopwatch;
    int m_renderTimeElapsed;

    Fps_sV m_fps;
    QSize m_resolution;
    FrameSize m_frameSize;
    InterpolationType m_interpolationType;

    bool m_initialized;
    bool m_stopRendering;
    qreal m_nextFrameTime;

    qreal m_prevTime;

    Qt::ConnectionType m_connectionType;

private slots:
    void slotRenderFrom(qreal time);

};

#endif // RENDERTASK_SV_H
