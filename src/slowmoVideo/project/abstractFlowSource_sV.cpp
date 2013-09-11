/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "abstractFlowSource_sV.h"

AbstractFlowSource_sV::AbstractFlowSource_sV(Project_sV *project) :
    m_project(project)
{
}

Project_sV* AbstractFlowSource_sV::project()
{
    return m_project;
}
