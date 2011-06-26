/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "v3dFlowSource_sV.h"
#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "../lib/flowRW_sV.h"

#include <QtCore/QProcess>
#include <QtCore/QSettings>

V3dFlowSource_sV::V3dFlowSource_sV(Project_sV *project, float lambda) :
    AbstractFlowSource_sV(project),
    m_lambda(lambda)
{
    m_dirFlowSmall = project->getDirectory("oFlowSmall");
    m_dirFlowOrig = project->getDirectory("oFlowOrig");

    m_dirFlowSmall.mkpath(".");
    m_dirFlowOrig.mkpath(".");
}

void V3dFlowSource_sV::slotUpdateProjectDir()
{
    m_dirFlowSmall.rmdir(".");
    m_dirFlowOrig.rmdir(".");

    m_dirFlowSmall = project()->getDirectory("oFlowSmall");
    m_dirFlowOrig = project()->getDirectory("oFlowOrig");

    m_dirFlowSmall.mkpath(".");
    m_dirFlowOrig.mkpath(".");
}

void V3dFlowSource_sV::setLambda(float lambda)
{
    m_lambda = lambda;
}

FlowField_sV* V3dFlowSource_sV::buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError)
{
    QSettings settings;
    QString programLocation(settings.value("binaries/v3dFlowBuilder", "/usr/local/bin/flowBuilder").toString());
    if (!QFile(programLocation).exists()) {
        throw FlowBuildingError("Program\n" + programLocation + "\ndoes not exist, cannot build flow!");
    }
    QString program(programLocation);
    QString flowFileName(flowPath(leftFrame, rightFrame, frameSize));

    qDebug() << "Flow target file: " << flowFileName;
    /// \todo Check if size is equal
    if (!QFile(flowFileName).exists()) {
        qDebug() << "Building flow for left frame " << leftFrame << " to right frame " << rightFrame << "; Size: " << frameSize;

        QStringList args;
        args    << project()->frameSource()->framePath(leftFrame, frameSize)
                << project()->frameSource()->framePath(rightFrame, frameSize)
                << flowFileName
                << QVariant(m_lambda).toString() << "100";

        qDebug() << "Arguments: " << args;

        QProcess proc;
        proc.start(program, args);
        proc.waitForFinished();
        if (proc.exitCode() != 0) {
            qDebug() << "Failed: " << proc.readAllStandardError() << proc.readAllStandardOutput();
            throw FlowBuildingError(QString("Flow builder exited with exit code %1; For details see debugging output").arg(proc.exitCode()));
        } else {
            qDebug() << "Optical flow built for " << flowFileName;
            qDebug() << proc.readAllStandardError() << proc.readAllStandardOutput();
        }
    } else {
        qDebug() << "Re-using existing flow image for left frame " << leftFrame << " to right frame " << rightFrame;
    }

    return FlowRW_sV::load(flowFileName.toStdString());
}

const QString V3dFlowSource_sV::flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const
{
    QDir dir;
    if (frameSize == FrameSize_Orig) {
        dir = m_dirFlowOrig;
    } else {
        dir = m_dirFlowSmall;
    }
    QString direction;
    if (leftFrame < rightFrame) {
        direction = "forward";
    } else {
        direction = "backward";
    }

    return dir.absoluteFilePath(QString("%1_%2-%3.sVflow").arg(direction).arg(leftFrame).arg(rightFrame));
}
