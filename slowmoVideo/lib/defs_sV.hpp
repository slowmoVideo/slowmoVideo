#ifndef DEFS_SV_HPP
#define DEFS_SV_HPP

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtGui/QColor>

#if _WIN64 || __amd64__
#define BITS_64
#endif

namespace Version_sV {
    static int major = 0;
    static int minor = 1;
    static QString version(QString("%1.%2").arg(major).arg(minor));
#ifdef BITS_64
    static QString bits("64-bit");
#else
    static QString bits("32-bit");
#endif
}

enum FlowDirection { FlowDirection_Forward, FlowDirection_Backward };
enum FrameSize { FrameSize_Orig = 1, FrameSize_Small = 2 };
enum CurveType { CurveType_Linear = 1, CurveType_Bezier = 2 };
enum TagAxis { TagAxis_Source = 1, TagAxis_Output = 2 };
enum InterpolationType { InterpolationType_Forward = 0, InterpolationType_ForwardNew = 1,
                         InterpolationType_Twoway = 10, InterpolationType_TwowayNew = 11,
                         InterpolationType_Bezier = 20 };

/// Default colours used in slowmoVideo (e.g. in the user interface)
namespace Colours_sV {
    static QColor colOk(158, 245, 94);
    static QColor colBad(247, 122, 48);
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
