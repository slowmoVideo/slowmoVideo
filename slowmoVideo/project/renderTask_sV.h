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
  \todo Segfault on emtpy project
  */
class RenderTask_sV : public QObject
{
    Q_OBJECT
public:
    RenderTask_sV(const Project_sV *project);
    ~RenderTask_sV();

    /** Manages the \c renderTarget pointer (includes destruction). */
    void setRenderTarget(AbstractRenderTarget_sV *renderTarget);
    void setTimeRange(float start, float end);
    void setFPS(float fps);
    void setSize(FrameSize size);

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

    float m_fps;
    FrameSize m_frameSize;

    bool m_stopRendering;
    qreal m_nextFrameTime;

private slots:
    void slotRenderFrom(qreal time);

};

#endif // RENDERTASK_SV_H
