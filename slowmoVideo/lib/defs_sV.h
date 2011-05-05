#ifndef DEFS_SV_H
#define DEFS_SV_H

#include <QDebug>

enum FlowDirection { FlowDirection_Forward, FlowDirection_Backward };

inline QDebug operator<<(QDebug qd, const FlowDirection &direction) {
    switch (direction) {
    case FlowDirection_Forward:
        qd << "Forward";
        break;
    case FlowDirection_Backward:
        qd << "Backward";
        break;
    }
    return qd;
}

#endif // DEFS_SV_H
