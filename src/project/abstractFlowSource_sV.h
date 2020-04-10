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

#include <QtCore>
#include <QDir>

class Project_sV;
class FlowField_sV;

class AbstractFlowSource_sV
{
public:
    AbstractFlowSource_sV(Project_sV *project);
    virtual ~AbstractFlowSource_sV() {}

    /** \return The flow field from \c leftFrame to \c rightFrame */
    virtual FlowField_sV* buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError) = 0;
    /** \return The path to the flow file for the given frames */
    virtual const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize = FrameSize_Orig) const = 0;

    virtual void setLambda(double lambda) { m_lambda = lambda;} ;

    void clearFlowCache();
    void createDirectories();
    void cleardirectory(QDir dir);

public slots:
    /**
      \fn slotUpdateProjectDir()
      Informs the flow source that the project directory has changed. If the flow source created sub-directories
      in the old project directories, it can e.g. delete them and create them at the new place.
      */
    virtual void slotUpdateProjectDir() ;

protected:
    Project_sV* project();
    double m_lambda;

    QDir m_dirFlowSmall;
    QDir m_dirFlowOrig;

private:
    Project_sV *m_project;

};

#endif // ABSTRACTFLOWSOURCE_SV_H
