/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef OPTICALFLOWBUILDER_SV_H
#define OPTICALFLOWBUILDER_SV_H

#include <QFile>

#include "defs_sV.hpp"

class OpticalFlowBuilder_sV {

public:
    virtual ~OpticalFlowBuilder_sV() {}

    virtual void buildFlow(const QFile& left, const QFile& right, const QFile& output, FlowDirection direction) const throw(FlowBuildingError) = 0;
};

#endif // OPTICALFLOWBUILDER_SV_H
