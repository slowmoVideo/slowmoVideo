/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef OPTICALFLOWBUILDERGPUKLT_SV_H
#define OPTICALFLOWBUILDERGPUKLT_SV_H

#include "opticalFlowBuilder_sV.h"

class OpticalFlowBuilderGPUKLT_sV : public OpticalFlowBuilder_sV
{
public:
    virtual void buildFlow(const QFile &left, const QFile &right, const QFile &output, FlowDirection direction) const;
};

#endif // OPTICALFLOWBUILDERGPUKLT_SV_H
