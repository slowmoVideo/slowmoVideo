/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef ABSTRACTFLOWSOURCE_SV_H
#define ABSTRACTFLOWSOURCE_SV_H

#include "../lib/defs_sV.hpp"

class Project_sV;
class FlowField_sV;

class AbstractFlowSource_sV
{
public:
    AbstractFlowSource_sV(Project_sV *project);

    virtual FlowField_sV* buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError) = 0;
    virtual const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize = FrameSize_Orig) const = 0;

public slots:
    virtual void slotUpdateProjectDir() = 0;

protected:
    Project_sV* project();

private:
    Project_sV *m_project;
};

#endif // ABSTRACTFLOWSOURCE_SV_H
