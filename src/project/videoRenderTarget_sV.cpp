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
#include <QtCore/QObject>

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
void VideoRenderTarget_sV::setVcodec(const QString &codec)
{
    m_vcodec = codec;
}

void VideoRenderTarget_sV::openRenderTarget() noexcept(false)
{
    char *vcodec = NULL;
    if (m_vcodec.length() > 0) {
        vcodec = (char*)malloc(m_vcodec.length()+1);
        strcpy(vcodec, m_vcodec.toStdString().c_str());
    }
    int worked =
    prepare(m_videoOut, m_filename.toStdString().c_str(), vcodec,
            renderTask()->resolution().width(), renderTask()->resolution().height(),
            renderTask()->fps().fps() * renderTask()->resolution().width() * renderTask()->resolution().height(),
            renderTask()->fps().den, renderTask()->fps().num);
    if (worked != 0) {
        throw Error_sV(QObject::tr("Video could not be prepared (error code %1).\n%2").arg(worked).arg(m_videoOut->errorMessage));
    }
}

void VideoRenderTarget_sV::slotConsumeFrame(const QImage &image, const int frameNumber)
{
    eatARGB(m_videoOut, image.bits());
}

void VideoRenderTarget_sV::closeRenderTarget() noexcept(false)
{
    finish(m_videoOut);
}
