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
#include "abstractFlowSource_sV.h"
#include "interpolator_sV.h"
#include "renderTask_sV.h"
#include "../lib/flowField_sV.h"
#include "../lib/shutter_sV.h"

#define MAX_CONV_FRAMES 5
//#define CONVOLUTION_BLUR

MotionBlur_sV::MotionBlur_sV(Project_sV *project) :
    m_project(project),
    m_slowmoSamples(16),
    m_maxSamples(64),
    m_slowmoMaxFrameDist(.5)
{
    createDirectories();
}

QImage MotionBlur_sV::blur(float startFrame, float endFrame, float replaySpeed, FrameSize size) throw(RangeTooSmallError_sV)
{
#ifdef CONVOLUTION_BLUR
    return convolutionBlur(startFrame, endFrame, replaySpeed, size);
#else
    if (replaySpeed > 0.5) {
        return fastBlur(startFrame, endFrame, size);
    } else {
        return slowmoBlur(startFrame, endFrame, size);
    }
#endif
}

QImage MotionBlur_sV::fastBlur(float startFrame, float endFrame, FrameSize size) throw(RangeTooSmallError_sV)
{
    float low, high;
    if (startFrame < endFrame) {
        low = startFrame; high = endFrame;
    } else {
        low = endFrame; high = startFrame;
    }
    low = qMax(low, float(0));
    high = qMin(high, (float)m_project->frameSource()->framesCount()-1);

    float dist = 0.125;
    float lowRounded;
    float highRounded;
    for (; ; dist *= 2) {
        lowRounded = ceil(low/dist)*dist;
        highRounded = floor(high/dist)*dist;
        if ((highRounded-lowRounded)/dist <= m_maxSamples) {
            break;
        }
    }

    float pos = lowRounded;

    QStringList frameList;

    while (pos < high) {
        frameList << cachedFramePath(pos, size);
        pos += dist;
    }
    qDebug() << "Fast blurring " << frameList.size() << " frames from " << startFrame << " to " << endFrame << ", low: " << low
             << ", high: " << high << ", with a distance of " << dist;

    if (frameList.size() > 1) {
        return Shutter_sV::combine(frameList);
    } else {
        throw RangeTooSmallError_sV(QString("Range too small: Start frame is %1, end frame is %2. "
                                            "Using normal interpolation.").arg(startFrame).arg(endFrame));
    }

}

/// \todo fixed distance as additional option
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
    float increment = (high-low)/(m_slowmoSamples-1);
    if (increment < .1) {
        qDebug() << "slowmoBlur(): Increasing distance from " << increment << " to .1";
        increment = .1;
    }
    if (increment > m_slowmoMaxFrameDist) {
        qDebug() << "slowmoBlur(): Decreasing distance from " << increment << " to " << m_slowmoMaxFrameDist;
        increment = m_slowmoMaxFrameDist;
    }
    for (float pos = low; pos <= high; pos += increment) {
        frameList << cachedFramePath(pos, size, true);
    }

    return Shutter_sV::combine(frameList);
}

QImage MotionBlur_sV::convolutionBlur(float startFrame, float endFrame, float replaySpeed, FrameSize size)
{
    float low, high;
    if (startFrame < endFrame) {
        low = startFrame; high = endFrame;
    } else {
        low = endFrame; high = startFrame;
    }
    low = qMax(low, float(0));
    high = qMin(high, (float)m_project->frameSource()->framesCount());

    if (floor(low) == floor(high) && low > .01) {
        qDebug() << "Small shutter." << startFrame << endFrame;
        FlowField_sV *field = m_project->requestFlow(floor(low), floor(low)+1, size);
        QImage blur = Shutter_sV::convolutionBlur(Interpolator_sV::interpolate(m_project, startFrame, InterpolationType_TwowayNew, size),
                                                  field,
                                                  high-low,
                                                  low-floor(low));
        delete field;
        return blur;
    }

    QList<QImage> images;
    FlowField_sV *field;
    int start = floor(low);
    int end = ceil(high);
    int inc = 1;
    qDebug() << "Large shutter." << startFrame << endFrame << " -- replay speed is " << replaySpeed;
    qDebug() << "Additional parts: " << start << end;
    if (replaySpeed < 2) {
        if (low-start > .1) {
            qDebug() << "First part: " << start << low;
            field = m_project->requestFlow(start, start+1, size);
            images << Shutter_sV::convolutionBlur(Interpolator_sV::interpolate(m_project, startFrame, InterpolationType_TwowayNew, size),
                                                  field,
                                                  high-low,
                                                  low-floor(low));
            delete field;
            start++;
        }
        if (end-high > .1) {
            qDebug() << "Last part: " << end-1 << high;
            field = m_project->requestFlow(end-1, end, size);
            images << Shutter_sV::convolutionBlur(m_project->frameSource()->frameAt(end-1, size),
                                                  field,
                                                  1 + high-end);
            delete field;
            end--;
        }
        if (end-start > MAX_CONV_FRAMES) {
            inc = 2;
        }
    } else {
        // \todo Check increment value
        while ((end-start) / inc > MAX_CONV_FRAMES) {
            if (inc == 1) {
                inc = 2;
            } else {
                inc += 2;
            }
        }
        start = start - start%inc;
        end = end - end%inc;
        qDebug() << "Parts scaled to " << start << end << " with increment " << inc;
    }
    for (int f = start; f <= end; f += inc) {
        field = m_project->requestFlow(f, f+1, size);
        images << Shutter_sV::convolutionBlur(m_project->frameSource()->frameAt(f, size),
                                              field,
                                              inc);
        delete field;
    }
    return Shutter_sV::combine(images);
}

QString MotionBlur_sV::cachedFramePath(float framePos, FrameSize size, bool highPrecision)
{
    int precision = 2;
    if (highPrecision) { precision = 2; } /// \todo check precision
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
    if (fabs(framePos-int(framePos)) < MOTIONBLUR_PRECISION_LIMIT) {
        name = m_project->frameSource()->framePath(uint(framePos), size);
    } else {
        if (!QFileInfo(name).exists()) {
            qDebug() << name << " does not exist yet. Interpolating and saving to cache.";
            QImage frm = Interpolator_sV::interpolate(m_project, framePos, InterpolationType_TwowayNew, size);
            frm.save(name);
        }
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

void MotionBlur_sV::setSlowmoSamples(int slowmoSamples)
{
    m_slowmoSamples = slowmoSamples;
    Q_ASSERT(m_slowmoSamples > 0);
}

void MotionBlur_sV::setMaxSamples(int maxSamples)
{
    m_maxSamples = maxSamples;
    Q_ASSERT(m_maxSamples > 0);
}
