/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef DEFS_SV_HPP
#define DEFS_SV_HPP

#include "macros_sV.h"

#if defined(WINDOWS) && !defined(MXE)
typedef __int64 int64_t;
#else
#include <inttypes.h>
#endif

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <cmath>

#define SLOWMOVIDEO_VERSION_MAJOR 0
#define SLOWMOVIDEO_VERSION_MINOR 2
#define SLOWMOVIDEO_VERSION_MICRO 6


/// Contains information about this slowmoVideo version
namespace Version_sV {
    /// Major version number
    static int major = SLOWMOVIDEO_VERSION_MAJOR;
    /// Minor version number
    static int minor = SLOWMOVIDEO_VERSION_MINOR;
    /// Micro version number
    static int micro = SLOWMOVIDEO_VERSION_MICRO;
    /// Version number as string
    static QString version(QString("%1.%2.%3").arg(major).arg(minor).arg(micro));
    /// Architecture
    static QString bits(
#ifdef BITS_64
            "64-bit"
#else
            "32-bit"
#endif
                        );
    /// Platform
    static QString platform(
#if defined LINUX
            "Linux"
#elif defined OSX
            "OSX"
#elif defined WINDOWS
            "Windows"
#endif
            );
}

enum FlowDirection { FlowDirection_Forward, FlowDirection_Backward };
enum FrameSize { FrameSize_Orig = 1, FrameSize_Small = 2 };
enum CurveType { CurveType_Linear = 1, CurveType_Bezier = 2 };
enum TagAxis { TagAxis_Source = 1, TagAxis_Output = 2 };
enum InterpolationType { InterpolationType_Forward = 0, InterpolationType_ForwardNew = 1,
                         InterpolationType_Twoway = 10, InterpolationType_TwowayNew = 11,
                         InterpolationType_Bezier = 20 };
enum MotionblurType { MotionblurType_Stacking = 0, MotionblurType_Convolving = 10,
                      MotionblurType_Nearest = 20 };

/// Default colours used in slowmoVideo (e.g. in the user interface)
namespace Colours_sV {
    static QColor colOk(158, 245, 94); ///< For checked text fields that are OK
    static QColor colBad(247, 122, 48); ///< For checked text fields that are invalid
}


/// For general errors.
class Error_sV {
public:
    /// Creates a new error object with the given information message.
    Error_sV(QString msg);
    /// Returns the information message.
    QString message() const;
private:
    QString m_message;
};

/// FPS representation, can guess numerator/denominator from a float value.
struct Fps_sV {
    /// numerator
    int num;
    /// denominator
    int den;

    Fps_sV(int num, int den) throw(Error_sV); ///< den must be > 0.

    Fps_sV(float fps) throw(Error_sV); ///< Converts a float fps number to a fractional. 23.97 and 29.97 are detected.

    Fps_sV(QString fpsString) throw(Error_sV); ///< Accepts fps strings like 24000/1001 for 23.97 fps.

    QString toString() const;

    /// Frames per second as float.
    double fps() const {
        return double(num)/den;
    }
};
/// For errors related to building optical flow.
class FlowBuildingError : public Error_sV {
public:
    /// Default constructor.
    FlowBuildingError(QString msg);
};
/// For errors related to the frame source.
class FrameSourceError : public Error_sV {
public:
    /// Default constructor.
    FrameSourceError(QString msg);
};
class InterpolationError : public Error_sV {
public:
    /// Default constructor
    InterpolationError(QString msg);
};

QString toString(const QSize& size);
QString toString(const FrameSize &size);
QString toString(const FlowDirection &dir);
QString toString(const CurveType &curveType);
QString toString(const QPointF &p);
QString toString(const TagAxis &axis);
QString toString(const InterpolationType &interpolation);
QString toString(const MotionblurType &interpolation);

inline QDebug operator<<(QDebug qd, const FlowDirection &direction) {
    switch (direction) {
    case FlowDirection_Forward:
        qd << "Forward";
        break;
    case FlowDirection_Backward:
        qd << "Backward";
        break;
    default:
        qd << "Unknown direction";
        Q_ASSERT(false);
        break;
    }
    return qd;
}

inline QDebug operator<<(QDebug qd, const FrameSize &frameSize)
{
    switch(frameSize) {
    case FrameSize_Orig:
        qd << "Original frame size";
        break;
    case FrameSize_Small:
        qd << "Small frame size";
        break;
    default:
        qd << "Unknown frame size";
        Q_ASSERT(false);
        break;
    }
    return qd;
}

#endif // DEFS_SV_HPP
