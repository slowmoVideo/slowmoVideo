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

class V3dFlowSource_sV : public AbstractFlowSource_sV
{
public:
    /** Creates a new flow source using V3D optical flow */
    V3dFlowSource_sV(Project_sV *project, float lambda = 10);
    void setLambda(float lambda);


    FlowField_sV* buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError);
    /// \todo Make path based on lambda
    const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const;

public slots:
    void slotUpdateProjectDir();

private:
    QDir m_dirFlowSmall;
    QDir m_dirFlowOrig;

    float m_lambda;

    void createDirectories();

};

#endif // V3DFLOWSOURCE_SV_H
