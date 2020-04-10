/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef ABSTRACTRENDERTARGET_SV_H
#define ABSTRACTRENDERTARGET_SV_H

#include <QtGui/QImage>
#include "../lib/defs_sV.hpp"

class RenderTask_sV;

/** \brief Should represent a render target like video or an image sequence */
class AbstractRenderTarget_sV
{
public:
    AbstractRenderTarget_sV(RenderTask_sV *parentRenderTask);
    virtual ~AbstractRenderTarget_sV();

    RenderTask_sV* renderTask() { return m_renderTask; }

    /// Prepares the render target (if necessary), like initializing video streams etc.
    virtual void openRenderTarget() throw(Error_sV)  = 0;
    /// Finishes the render target (e.g. writes the trailer to a video file)
    virtual void closeRenderTarget() throw(Error_sV) = 0;

public slots:
    /// Adds one frame to the output
    virtual void slotConsumeFrame(const QImage &image, const int frameNumber) = 0;

private:
    RenderTask_sV *m_renderTask;

};

#endif // ABSTRACTRENDERTARGET_SV_H
