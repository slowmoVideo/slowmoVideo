/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef IMAGESFRAMESOURCE_SV_H
#define IMAGESFRAMESOURCE_SV_H

#include "abstractFrameSource_sV.h"
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QSize>

class Project_sV;

/**
  \todo Allow re-ordering of images
  */
class ImagesFrameSource_sV : public AbstractFrameSource_sV
{
    Q_OBJECT
public:
    ImagesFrameSource_sV(Project_sV *project, QStringList images) throw(FrameSourceError);

    static QString validateImages(const QStringList images);

    void initialize();
    bool initialized() const;

    int64_t framesCount() const;
    const Fps_sV* fps() const;
    QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig);
    const QString framePath(const uint frame, const FrameSize frameSize) const;

    const QStringList inputFiles() const;


public slots:
    void slotAbortInitialization();
    void slotUpdateProjectDir();

private:
    QStringList m_imagesList;
    QDir m_dirImagesSmall;
    QSize m_sizeSmall;

    Fps_sV m_fps;

    bool m_initialized;
    bool m_stopInitialization;
    int m_nextFrame;

    void createDirectories();


private slots:
    void slotContinueInitialization();

};

#endif // IMAGESFRAMESOURCE_SV_H
