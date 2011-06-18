/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "abstractFrameSource_sV.h"
#include "flow_sV.h"
#include "../lib/opticalFlowBuilderGPUKLT_sV.h"

#include <QFile>
#include <QDebug>


#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

Flow_sV::Flow_sV()
{
    m_flowBuilder = new OpticalFlowBuilderGPUKLT_sV();
}

Flow_sV::~Flow_sV()
{
    delete m_flowBuilder;
}

void Flow_sV::buildFlow(Project_sV *project, ProjectFrameMemFn frameNames, ProjectFlowMemFn outName, FlowDirection direction) throw(FlowBuildingError)
{
    m_abort = false;
    m_aborted = false;
    int framesCount = project->frameSource()->framesCount();

    for (int i = 2; i < framesCount; i++) {
        if (m_abort) {
            m_aborted = true;
            break;
        }
        QString left, right, flowFile;
        left = CALL_MEMBER_FN(project, frameNames)(i-1);
        right = CALL_MEMBER_FN(project, frameNames)(i);
        flowFile = CALL_MEMBER_FN(project, outName)(i-1, direction);

        emit signalFlowFrame(flowFile);

        if (!QFile(flowFile).exists()) {
            buildFlowImage(left, right, flowFile, direction);
        } else {
            qDebug() << flowFile << " already exists.";
        }
        emit signalFlowProgressUpdated(i);

    }
    if (m_aborted) {
        emit signalFlowAborted();
    } else {
        emit signalFlowFinished();
    }
}

void Flow_sV::buildFlowImage(const QString &leftFrame, const QString &rightFrame, const QString &outFrame, FlowDirection direction) throw(FlowBuildingError)
{
    m_flowBuilder->buildFlow(leftFrame, rightFrame, outFrame, direction);
}

void Flow_sV::slotAbort()
{
    m_abort = true;
}
