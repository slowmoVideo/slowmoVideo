#ifndef OPTICALFLOWBUILDERGPUKLT_SV_H
#define OPTICALFLOWBUILDERGPUKLT_SV_H

#include "opticalFlowBuilder_sV.h"

class OpticalFlowBuilderGPUKLT_sV : public OpticalFlowBuilder_sV
{
public:
    virtual void buildFlow(const QFile &left, const QFile &right, const QFile &output, FlowDirection direction) const;
};

#endif // OPTICALFLOWBUILDERGPUKLT_SV_H
