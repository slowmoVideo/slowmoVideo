/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef IMAGESRENDERTARGET_SV_H
#define IMAGESRENDERTARGET_SV_H

#include "abstractRenderTarget_sV.h"

#include <QtCore/QDir>

class RenderTask_sV;

class ImagesRenderTarget_sV : public AbstractRenderTarget_sV
{
public:
    ImagesRenderTarget_sV(RenderTask_sV *parentRenderTask);

    virtual void openRenderTarget() throw(Error_sV) {} ;
    virtual void closeRenderTarget() throw(Error_sV) {} ;


    void setTargetDir(const QDir dir);
    bool setFilenamePattern(const QString pattern);

public slots:
    void slotConsumeFrame(const QImage &image, const int frameNumber);

private:
    QDir m_targetDir;
    QString m_filenamePattern;
};

#endif // IMAGESRENDERTARGET_SV_H
