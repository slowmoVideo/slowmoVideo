/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "emptyFrameSource_sV.h"

EmptyFrameSource_sV::EmptyFrameSource_sV(const Project_sV *project) :
    AbstractFrameSource_sV(project),
    m_fps(24,1)
{
}

void EmptyFrameSource_sV::initialize()
{
    emit signalAllTasksFinished();
}
