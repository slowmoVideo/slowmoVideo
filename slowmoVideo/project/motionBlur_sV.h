/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef MOTIONBLUR_SV_H
#define MOTIONBLUR_SV_H

#include <QtCore/QDir>
#include <QtGui/QImage>
#include "../lib/defs_sV.hpp"
class Project_sV;

class MotionBlur_sV
{
public:
    MotionBlur_sV(Project_sV *project, int minSamples);

    QImage blur(float startFrame, float endFrame, float replaySpeed, FrameSize size);

    QImage fastBlur(float startFrame, float endFrame, FrameSize size);

    QImage slowmoBlur(float startFrame, float endFrame, FrameSize size);

public slots:
    void slotUpdateProjectDir();

private:
    Project_sV *m_project;
    QDir m_dirCacheSmall;
    QDir m_dirCacheOrig;

    int m_minSamples;

    QString cachedFramePath(float framePos, FrameSize size, bool highPrecision = false);
    void createDirectories();
};

#endif // MOTIONBLUR_SV_H
