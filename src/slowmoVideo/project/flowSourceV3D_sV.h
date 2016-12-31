/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef V3DFLOWSOURCE_SV_H
#define V3DFLOWSOURCE_SV_H

#include "abstractFlowSource_sV.h"

#include <QtCore/QDir>

class FlowSourceV3D_sV : public AbstractFlowSource_sV
{
public:
    /** Creates a new flow source using V3D optical flow */
    FlowSourceV3D_sV(Project_sV *project, double lambda = 10);
    ~FlowSourceV3D_sV() {}


    FlowField_sV* buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError);
    const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const;

    static bool validateFlowBinary(const QString path);
    static QString correctFlowBinaryLocation();

private:

};

#endif // V3DFLOWSOURCE_SV_H
