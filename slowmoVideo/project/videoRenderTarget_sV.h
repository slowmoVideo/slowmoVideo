/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef VIDEORENDERTARGET_SV_H
#define VIDEORENDERTARGET_SV_H

#include "abstractRenderTarget_sV.h"

class VideoOut_sV;
class RenderTask_sV;

/// Produces videos from frames.
class VideoRenderTarget_sV : public AbstractRenderTarget_sV
{
public:
    VideoRenderTarget_sV(RenderTask_sV *parentRenderTask);
    virtual ~VideoRenderTarget_sV();

    /// openRenderTarget() will throw an error if the target file cannot be opened.
    void setTargetFile(const QString& filename);

    void openRenderTarget() throw(Error_sV);
    void closeRenderTarget() throw(Error_sV);

public slots:
    void slotConsumeFrame(const QImage &image, const int frameNumber);

private:
    QString m_filename;
    VideoOut_sV *m_videoOut;

    int m_width;
    int m_height;
};

#endif // VIDEORENDERTARGET_SV_H
