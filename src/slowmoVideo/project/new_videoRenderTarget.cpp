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

#include "new_videoRenderTarget.h"
#include "renderTask_sV.h"
#include <QtCore/QObject>

#include "../lib/video_enc.h"

newVideoRenderTarget::newVideoRenderTarget(RenderTask_sV *parentRenderTask) :
    AbstractRenderTarget_sV(parentRenderTask)
{
    
}
newVideoRenderTarget::~newVideoRenderTarget()
{
    
}

void newVideoRenderTarget::setTargetFile(const QString &filename)
{
    m_filename = filename;
}
void newVideoRenderTarget::setVcodec(const QString &codec)
{
    m_vcodec = codec;
}

void newVideoRenderTarget::openRenderTarget() throw(Error_sV)
{
    writer = CreateVideoWriter(m_filename.toStdString().c_str(),
    	renderTask()->resolution().width(),
    	renderTask()->resolution().height(),
    	renderTask()->fps().fps(),1);
    
   
    if (writer == 0) {
        throw Error_sV(QObject::tr("Video could not be prepared .\n"));
    }
}

void newVideoRenderTarget::slotConsumeFrame(const QImage &image, const int frameNumber)
{
    WriteFrame(writer, image);
}

void newVideoRenderTarget::closeRenderTarget() throw(Error_sV)
{
    ReleaseVideoWriter( &writer );
}
