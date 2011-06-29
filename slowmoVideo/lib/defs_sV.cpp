#include "defs_sV.hpp"

Error_sV::Error_sV(QString msg) :
    m_message(msg) {}

QString Error_sV::message() const {
    return m_message;
}

FlowBuildingError::FlowBuildingError(QString msg) :
    Error_sV(msg) {}
FrameSourceError::FrameSourceError(QString msg) :
    Error_sV(msg) {}

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

QString toString(const NodeContext &context)
{
    switch (context) {
    case NodeContext_Node:
        return QString("Node");
    case NodeContext_Segment:
        return QString("Segment");
    case NodeContext_Handle:
        return QString("Handle");
    case NodeContext_None:
        return QString("None");
    default:
        Q_ASSERT(false);
        return QString("Unknown context");
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
