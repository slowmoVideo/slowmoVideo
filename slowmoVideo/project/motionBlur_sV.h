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

/// Thrown if the frame range is too small for motion blur to still make sense
class RangeTooSmallError_sV : public Error_sV {
public:
    RangeTooSmallError_sV(QString msg) : Error_sV(msg) {}
};

#define MOTIONBLUR_PRECISION_LIMIT .001

/**
  \brief Renders motion blur
  \todo Force fast/slow blurring for a segment?
  */
class MotionBlur_sV
{
public:
    MotionBlur_sV(Project_sV *project);

    /**
      Selects either fastBlur() or slowmoBlur(), depending on the replay speed.
      */
    QImage blur(float startFrame, float endFrame, float replaySpeed, FrameSize size) throw(RangeTooSmallError_sV);

    /**
      Blurs frames using cached frames on fixed, coarse-grained intervals.
      If the replay speed is high enough, it does not matter if frame 1.424242 or frame 1.5 is used
      together with other frames for rendering motion blur. That way calculation can be sped up a little bit.
      */
    QImage fastBlur(float startFrame, float endFrame, FrameSize size) throw(RangeTooSmallError_sV);

    /**
      Blurs frames that are re-played at very low speed, such that fastBlur() cannot be used.
      The blurred parts of the image still need to move slowly, rounding frames to interpolate to 0.5
      would not work therefore.
      */
    QImage slowmoBlur(float startFrame, float endFrame, FrameSize size);

    /**
      \fn setSlowmoSamples();
      Sets the minimum number of samples for motion blur. This is ignored by fastBlur() where the interpolation scale
      is fixed (i.e. at most 1/8 steps between two frames). However slowmoBlur() uses this exact value for interpolating.
      */
    /**
      \fn setMaxSamples();
      Sets the maximum number of samples that are used for rendering motion blur.
      */
    void setSlowmoSamples(int slowmoSamples);
    void setMaxSamples(int maxSamples);
    void setSlowmoMaxFrameDistance(float distance);

    int slowmoSamples() const { return m_slowmoSamples; }
    int maxSamples() const { return m_maxSamples; }

public slots:
    void slotUpdateProjectDir();

private:
    Project_sV *m_project;
    QDir m_dirCacheSmall;
    QDir m_dirCacheOrig;

    int m_slowmoSamples;
    int m_maxSamples;
    float m_slowmoMaxFrameDist;

    QString cachedFramePath(float framePos, FrameSize size, bool highPrecision = false);
    void createDirectories();
};

#endif // MOTIONBLUR_SV_H
