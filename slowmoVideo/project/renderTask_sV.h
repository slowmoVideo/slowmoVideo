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

#include <QObject>

#include "../lib/defs_sV.hpp"

class Project_sV;
class AbstractRenderTarget_sV;

/**
  \brief Renders a project when started.
  \todo Changes in the project affect the rendering as well. Copy the project?
  \todo Time range for rendering (markers?)
  */
class RenderTask_sV : public QObject
{
    Q_OBJECT
public:
    RenderTask_sV(const Project_sV *project);
    ~RenderTask_sV();

    /**
      \fn setRenderTarget()
      Manages the \c renderTarget pointer (includes destruction).
      */
    /**
      \fn setTimeRange()
      Sets the time range for rendering. By default, the whole project is rendered.
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
    void setTimeRange(float start, float end);
    void setFPS(float fps);
    void setSize(FrameSize size);
    void setInterpolationType(const InterpolationType interpolation);

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
    void signalRenderingStopped();
    void signalRenderingFinished();
    void signalRenderingAborted(QString reason);

    void signalFrameRendered(qreal time, int frameNumber);

private:
    const Project_sV *m_project;
    AbstractRenderTarget_sV *m_renderTarget;

    float m_timeStart;
    float m_timeEnd;

    Fps_sV m_fps;
    QSize m_resolution;
    FrameSize m_frameSize;
    InterpolationType m_interpolationType;

    bool m_initialized;
    bool m_stopRendering;
    qreal m_nextFrameTime;

    qreal m_prevTime;

private slots:
    void slotRenderFrom(qreal time);

};

#endif // RENDERTASK_SV_H
