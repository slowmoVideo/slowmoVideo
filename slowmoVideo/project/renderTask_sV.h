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
class RenderTask_sV : public QObject
{
    Q_OBJECT
public:
    RenderTask_sV(const Project_sV *project);

public slots:
    void slotAbortRendering();
    void slotContinueRendering(qreal time = -1);
//    void slotUpdateRenderFrameSize(const FrameSize frameSize);

signals:
    void signalFrameRendered(qreal time, int frameNumber);
    void signalRenderingFinished();
    void signalRenderingAborted();

private:
    const Project_sV *m_project;

//    FrameSize m_frameSize;
    bool m_stopRendering;
    qreal m_nextFrameTime;

private slots:
    void slotRenderFrom(qreal time);

};

#endif // RENDERTASK_SV_H
