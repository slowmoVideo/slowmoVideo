#ifndef DEFS_SV_HPP
#define DEFS_SV_HPP

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtGui/QColor>

enum FlowDirection { FlowDirection_Forward, FlowDirection_Backward };
enum FrameSize { FrameSize_Orig = 1, FrameSize_Small = 2 };
enum CurveType { CurveType_Linear = 1, CurveType_Bezier = 2 };
enum TagAxis { TagAxis_Source = 1, TagAxis_Output = 2 };

namespace Colours_sV {
    static QColor colOk(158, 245, 94);
    static QColor colBad(247, 122, 48);
}

class Error_sV {
public:
    Error_sV(QString msg);
    QString message() const;
private:
    QString m_message;
};
class FlowBuildingError : public Error_sV {
public: FlowBuildingError(QString msg);
};
class FrameSourceError : public Error_sV {
public: FrameSourceError(QString msg);
};

QString toString(const QSize& size);
QString toString(const FrameSize &size);
QString toString(const FlowDirection &dir);
QString toString(const CurveType &curveType);
QString toString(const QPointF &p);
QString toString(const TagAxis &axis);

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
