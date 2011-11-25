#include "defs_sV.hpp"

Error_sV::Error_sV(QString msg) :
    m_message(msg)
{
    qDebug() << msg;
}

QString Error_sV::message() const {
    return m_message;
}

FlowBuildingError::FlowBuildingError(QString msg) :
    Error_sV(msg)
{
    qDebug() << "Flow building error: " << msg;
}
FrameSourceError::FrameSourceError(QString msg) :
    Error_sV(msg)
{
    qDebug() << "Frame source error: " << msg;
}
InterpolationError::InterpolationError(QString msg) :
    Error_sV(msg)
{
    qDebug() << "Interpolation error : " << msg;
}

QString toString(const QSize &size)
{
    return QString::fromUtf8("%1×%2").arg(size.width()).arg(size.height());
}

QString toString(const FrameSize &size)
{
    switch (size) {
    case FrameSize_Orig:
        return QString("Orig");
    case FrameSize_Small:
        return QString("Small");
    default:
        Q_ASSERT(false);
        return QString("Unknown size");
    }
}

QString toString(const FlowDirection &dir)
{
    switch (dir) {
    case FlowDirection_Forward:
        return QString("Forward");
    case FlowDirection_Backward:
        return QString("Backward");
    default:
        Q_ASSERT(false);
        return QString("Unknown direction");
    }
}

QString toString(const CurveType &curveType)
{
    switch (curveType) {
    case CurveType_Linear:
        return QString("Linear");
    case CurveType_Bezier:
        return QString::fromUtf8("Bézier");
    default:
        Q_ASSERT(false);
        return QString("Unknown curve type");
    }
}

QString toString(const QPointF &p)
{
    return QString("(%1|%2)").arg(p.x()).arg(p.y());
}

QString toString(const TagAxis axis)
{
    switch (axis) {
    case TagAxis_Source:
        return QString("Source axis");
    case TagAxis_Output:
        return QString("Output axis");
    default:
        Q_ASSERT(false);
        return QString("Unknown axis");
    }
}

QString toString(const InterpolationType &interpolation)
{
    switch (interpolation) {
    case InterpolationType_Forward:
        return QString("Forward interpolation (fast)");
    case InterpolationType_ForwardNew:
        return QString("Forward interpolation (accurate)");
    case InterpolationType_Twoway:
        return QString("Two-way interpolation (fast)");
    case InterpolationType_TwowayNew:
        return QString("Two-way interpolation (accurate)");
    case InterpolationType_Bezier:
        return QString::fromUtf8("Bézier interpolation");
    default:
        Q_ASSERT(false);
        return QString("Unknown interpolation");
    }
}

QString toString(const MotionblurType &type)
{
    switch (type) {
    case MotionblurType_Stacking:
        return QString("Stacking");
    case MotionblurType_Convolving:
        return QString("Convolution");
    case MotionblurType_Nearest:
        return QString("Nearest (no blurring)");
    default:
        Q_ASSERT(false);
        return QString("Unknown motion blur type");
    }
}

Fps_sV::Fps_sV(int num, int den) throw(Error_sV) :
    num(num), den(den)
{
    if (den <= 0) {
        throw Error_sV("FPS denominator must be >= 0.");
    }
}
Fps_sV::Fps_sV(QString fpsString) throw(Error_sV)
{
    QRegExp e("(\\d+)\\/(\\d+)");
    if (e.exactMatch(fpsString)) {
        num = e.cap(1).toInt();
        den = e.cap(2).toInt();
        if (den <= 0) {
            throw Error_sV("FPS denominator must be >= 0.");
        }
    } else {
        throw Error_sV("Cannot create fps value from " + fpsString);
    }
}
Fps_sV::Fps_sV(float fps) throw(Error_sV)
{
    if (fps <= 0) {
        throw Error_sV(QString("FPS value must be larger than zero (is: %1)").arg(fps));
    }
    // Check for 23.976 and similar numbers (24*1000/1001)
    if (fabs(1000*ceil(fps)-1001*fps) < 7) {
        num = 1000*ceil(fps);
        den = 1001;
    } else {
        num = 100000*fps;
        den = 100000;
        // Prettify
        for (int i = 10; i > 1; i--) {
            while (num % i == 0 && den % i == 0) {
                num /= i;
                den /= i;
            }
        }
    }
}

QString Fps_sV::toString() const
{
    return QString("%1/%2").arg(num).arg(den);
}
