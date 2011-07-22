/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "motionBlur_sV.h"
#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "interpolator_sV.h"
#include "../lib/shutter_sV.h"

MotionBlur_sV::MotionBlur_sV(Project_sV *project, int minSamples) :
    m_project(project),
    m_minSamples(minSamples)
{
    createDirectories();
}

QImage MotionBlur_sV::blur(float startFrame, float endFrame, float replaySpeed, FrameSize size)
{
    if (replaySpeed > 0.5) {
        return fastBlur(startFrame, endFrame, size);
    } else {
        return slowmoBlur(startFrame, endFrame, size);
    }
}

QImage MotionBlur_sV::fastBlur(float startFrame, float endFrame, FrameSize size)
{
    /// \todo Check if endFrame-startFrame large enough!
    /// \todo m_minSamples


    float low, high;
    if (startFrame < endFrame) {
        low = startFrame; high = endFrame;
    } else {
        low = endFrame; high = startFrame;
    }
    low = qMax(low, float(0));
    high = qMin(high, (float)m_project->frameSource()->framesCount());

    float pos = ceil(4*low)/4.0;    // Round to quarters

    QStringList frameList;

    qDebug() << "start: " << startFrame << ", end: " << endFrame << ", low: " << low
             << ", high: " << high << ", pos: " << pos;
    while (pos < high) {
        qDebug() << "pos: " << pos;
        frameList << cachedFramePath(pos, size);
        pos += .25;
    }

    return Shutter_sV::combine(frameList);
}

QImage MotionBlur_sV::slowmoBlur(float startFrame, float endFrame, FrameSize size)
{
    float low, high;
    if (startFrame < endFrame) {
        low = startFrame; high = endFrame;
    } else {
        low = endFrame; high = startFrame;
    }
    low = qMax(low, float(0));
    high = qMin(high, (float)m_project->frameSource()->framesCount());

    QStringList frameList;
    float increment = (high-low)/(m_minSamples-1);
    for (float pos = low; pos <= high; pos += increment) {
        qDebug() << "pos: " << pos;
        frameList << cachedFramePath(pos, size, true);
    }

    return Shutter_sV::combine(frameList);
}

QString MotionBlur_sV::cachedFramePath(float framePos, FrameSize size, bool highPrecision)
{
    int precision = 2;
    if (highPrecision) { precision = 6; }
    int width = 5+1 + precision;
    QString name = QString("%2/cached%1.png").arg(framePos, width, 'f', precision, '0');
    if (size == FrameSize_Small) {
        name = name.arg(m_dirCacheSmall.absolutePath());
    } else if (size == FrameSize_Orig) {
        name = name.arg(m_dirCacheOrig.absolutePath());
    } else {
        qDebug() << "MotionBlur: Frame size " << toString(size) << " given, not supported!";
        Q_ASSERT(false);
    }
    if (!QFileInfo(name).exists()) {
        qDebug() << name << " does not exist yet. Interpolating.";
        /// \todo
        QImage frm = Interpolator_sV::interpolate(m_project, framePos, InterpolationType_TwowayNew, size);
        frm.save(name);
    } else {
        qDebug() << "Frame is cached already: " << name;
    }
    return name;
}

void MotionBlur_sV::slotUpdateProjectDir()
{
    m_dirCacheSmall.rmdir(".");
    m_dirCacheOrig.rmdir(".");
    createDirectories();
}


void MotionBlur_sV::createDirectories()
{
    m_dirCacheSmall = m_project->getDirectory("cache/motionBlurSmall");
    m_dirCacheOrig = m_project->getDirectory("cache/motionBlurOrig");
}
