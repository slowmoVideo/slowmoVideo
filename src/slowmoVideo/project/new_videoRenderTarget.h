/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

/*
 * handle both ffmpeg and qtkit
 */
#ifndef NEW_VIDEORENDERTARGET_SV_H
#define NEW_VIDEORENDERTARGET_SV_H

#include "abstractRenderTarget_sV.h"

class VideoWriter;
class RenderTask_sV;

/// Produces videos from frames.
class newVideoRenderTarget : public AbstractRenderTarget_sV
{
public:
    /// Constructs a new video render target
    newVideoRenderTarget(RenderTask_sV *parentRenderTask);
    virtual ~newVideoRenderTarget();

    /// openRenderTarget() will throw an error if the target file cannot be opened.
    void setTargetFile(const QString& filename);
    /// Set a custom video codec
    void setVcodec(const QString& codec);

    void openRenderTarget() throw(Error_sV);
    void closeRenderTarget() throw(Error_sV);

public slots:
    void slotConsumeFrame(const QImage &image, const int frameNumber);

private:
    QString m_filename;
    QString m_vcodec;
    VideoWriter* writer;

    int m_width;
    int m_height;
};

#endif // NEW_VIDEORENDERTARGET_SV_H
