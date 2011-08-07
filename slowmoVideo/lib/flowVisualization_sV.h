#ifndef FLOWVISUALIZATION_SV_H
#define FLOWVISUALIZATION_SV_H

#include "flowField_sV.h"
#include <QtGui/QImage>

class FlowVisualization_sV
{
public:
    static QImage colourizeFlow(const FlowField_sV *flowField, float boost = 1.0);
};

#endif // FLOWVISUALIZATION_SV_H
