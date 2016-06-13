/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "abstractFlowSource_sV.h"

#include <QtCore>
#include <QDir>

#include "project_sV.h"

AbstractFlowSource_sV::AbstractFlowSource_sV(Project_sV *project) :
    m_project(project)
{
}

Project_sV* AbstractFlowSource_sV::project()
{
    return m_project;
}

/*
 * clear all flow file in flow directories
 */
void AbstractFlowSource_sV::clearFlowCache()
{
	  //TODO: loop for small files also !
	  QDir dir = m_dirFlowOrig;

    dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
    foreach( QString dirItem, dir.entryList() )
        dir.remove( dirItem );

    dir.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    foreach( QString dirItem, dir.entryList() )
    {
        QDir subDir( dir.absoluteFilePath( dirItem ) );
        subDir.removeRecursively();
    }
}

void AbstractFlowSource_sV::slotUpdateProjectDir()
{
     //TODO: check
     //m_dirFlowSmall.rmdir(".");
     //m_dirFlowOrig.rmdir(".");
     createDirectories();
}


void AbstractFlowSource_sV::createDirectories()
{
     m_dirFlowSmall = project()->getDirectory("cache/oFlowSmall");
     m_dirFlowOrig = project()->getDirectory("cache/oFlowOrig");
}
