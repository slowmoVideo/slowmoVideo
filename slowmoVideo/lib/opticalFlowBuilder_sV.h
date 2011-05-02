#ifndef OPTICALFLOWBUILDER_SV_H
#define OPTICALFLOWBUILDER_SV_H

#include <QFile>

#include "defs_sV.h"

class OpticalFlowBuilder_sV {

public:
    virtual ~OpticalFlowBuilder_sV() {}

    virtual void buildFlow(const QFile& left, const QFile& right, const QFile& output, FlowDirection direction) const = 0;
};

#endif // OPTICALFLOWBUILDER_SV_H
