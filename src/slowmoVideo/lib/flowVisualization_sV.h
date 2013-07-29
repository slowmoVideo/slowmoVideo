#ifndef FLOWVISUALIZATION_SV_H
#define FLOWVISUALIZATION_SV_H

#include "flowField_sV.h"
#include <QtGui/QImage>

class FlowVisualization_sV
{
public:
    enum ColourizingType { WXY, HSV };
    static QImage colourizeFlow(const FlowField_sV *flowField, ColourizingType type, float boost = 1.0);

private:
    static QImage colourizeFlowWXY(const FlowField_sV *flowField, float boost = 1.0);
    static QImage colourizeFlowHSV(const FlowField_sV *flowField, float boost = 1.0);
};

#endif // FLOWVISUALIZATION_SV_H
