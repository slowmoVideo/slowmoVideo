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
