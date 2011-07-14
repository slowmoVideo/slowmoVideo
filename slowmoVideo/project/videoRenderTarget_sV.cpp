/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

// Against the «UINT64_C not declared» message.
// See: http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#include "videoRenderTarget_sV.h"
#include "renderTask_sV.h"

extern "C" {
#include "../lib/ffmpegEncode_sV.h"
}

VideoRenderTarget_sV::VideoRenderTarget_sV(RenderTask_sV *parentRenderTask) :
    AbstractRenderTarget_sV(parentRenderTask)
{
    m_videoOut = (VideoOut_sV*)malloc(sizeof(VideoOut_sV));
}
VideoRenderTarget_sV::~VideoRenderTarget_sV()
{
    free(m_videoOut);
}

void VideoRenderTarget_sV::setTargetFile(const QString &filename)
{
    m_filename = filename;
}

void VideoRenderTarget_sV::openRenderTarget() throw(Error_sV)
{
    int worked =
    prepare(m_videoOut, m_filename.toStdString().c_str(),
            renderTask()->resolution().width(), renderTask()->resolution().height(),
            renderTask()->fps() * renderTask()->resolution().width() * renderTask()->resolution().height(),
            1, renderTask()->fps());
    if (worked != 0) {
        throw Error_sV(QString("Video could not be prepared. \n%2 (%1)").arg(worked).arg(m_videoOut->errorMessage));
    }
}

void VideoRenderTarget_sV::slotConsumeFrame(const QImage &image, const int frameNumber)
{
    eatARGB(m_videoOut, image.bits());
}

void VideoRenderTarget_sV::closeRenderTarget() throw(Error_sV)
{
    finish(m_videoOut);
}
