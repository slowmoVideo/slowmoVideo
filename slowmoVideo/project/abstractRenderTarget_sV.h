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

#include <QImage>


/** \brief Should represent a render target like video or an image sequence */
class AbstractRenderTarget_sV
{
public:
    AbstractRenderTarget_sV();
    virtual ~AbstractRenderTarget_sV();

public slots:
    virtual void slotConsumeFrame(const QImage &image, const int frameNumber) = 0;

};

#endif // ABSTRACTRENDERTARGET_SV_H
