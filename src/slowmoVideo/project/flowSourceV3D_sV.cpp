/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowSourceV3D_sV.h"
#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "../lib/flowRW_sV.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QTime>

FlowSourceV3D_sV::FlowSourceV3D_sV(Project_sV *project, double lambda) :
    AbstractFlowSource_sV(project)
{
    createDirectories();
    m_lambda = lambda;
}


FlowField_sV* FlowSourceV3D_sV::buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError)
{
    QString flowFileName(flowPath(leftFrame, rightFrame, frameSize));

    /// \todo Check if size is equal
    if (!QFile(flowFileName).exists()) {
        QSettings settings;
        QString programLocation(settings.value("binaries/v3dFlowBuilder", "/usr/local/bin/slowmoFlowBuilder").toString());
        if (!QFile(programLocation).exists()) {
            QString newLoc = correctFlowBinaryLocation();
            if (newLoc.length() > 0) {
                programLocation = newLoc;
            }
        }
        if (!QFile(programLocation).exists()) {
            throw FlowBuildingError("Program\n" + programLocation + "\ndoes not exist (did  you compile/make V3D?), cannot build flow!");
        }
        QString program(programLocation);

        qDebug() << "Building flow for left frame " << leftFrame << " to right frame " << rightFrame << "; Size: " << frameSize;

        QStringList args;
        args    << project()->frameSource()->framePath(leftFrame, frameSize)
                << project()->frameSource()->framePath(rightFrame, frameSize)
                << flowFileName
                << QVariant(m_lambda).toString() << "100";

        qDebug() << "Arguments: " << args;


        QTime time;
        QProcess proc;

        time.start();
        proc.start(program, args);
        proc.waitForFinished(-1);
        if (proc.exitCode() != 0) {
            qDebug() << "Failed: " << proc.readAllStandardError() << proc.readAllStandardOutput();
            throw FlowBuildingError(QString("Flow builder exited with exit code %1; For details see debugging output").arg(proc.exitCode()));
        } else {
            qDebug() << "Optical flow built for " << flowFileName << " in " << time.elapsed() << " ms";
            qDebug() << proc.readAllStandardError() << proc.readAllStandardOutput();
        }
    } else {
        qDebug().nospace() << "Re-using existing flow image for left frame " << leftFrame << " to right frame " << rightFrame << ": " << flowFileName;
    }

    try {
        return FlowRW_sV::load(flowFileName.toStdString());
    } catch (FlowRW_sV::FlowRWError &err) {
        throw FlowBuildingError(err.message.c_str());
    }
}



QString FlowSourceV3D_sV::correctFlowBinaryLocation()
{
    QSettings settings;
    QString programLocation(settings.value("binaries/v3dFlowBuilder", "/usr/local/bin/slowmoFlowBuilder").toString());

    QStringList paths;
    paths << programLocation;
    paths << QDir::currentPath() + "/slowmoFlowBuilder";
    paths << QCoreApplication::applicationDirPath() + "/slowmoFlowBuilder";
    paths << "/usr/bin/slowmoFlowBuilder" << "/usr/local/bin/slowmoFlowBuilder";
    for (int i = 0; i < paths.size(); i++) {
        if (validateFlowBinary(paths.at(i))) {
            settings.setValue("binaries/v3dFlowBuilder", paths.at(i));
            return paths.at(i);
        }
    }
    return QString();
}

bool FlowSourceV3D_sV::validateFlowBinary(const QString path)
{
    bool valid = false;
    qDebug() << "Checking " << path << " ...";
    if (QFile(path).exists() && QFileInfo(path).isExecutable()) {
        QProcess process;
        QStringList args;
        args << "--identify";
        process.start(path, args);
        process.waitForFinished(2000);
        QString output(process.readAllStandardOutput());
        if (output.startsWith("slowmoFlowBuilder")) {
            valid = true;
            qDebug() << path << " is valid.";
        } else {
            qDebug() << "Invalid output from flow executable: " << output;
        }
        process.terminate();
    }
    return valid;
}


const QString FlowSourceV3D_sV::flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const
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

    return dir.absoluteFilePath(QString("%1-lambda%4_%2-%3.sVflow").arg(direction).arg(leftFrame).arg(rightFrame).arg(m_lambda, 0, 'f', 2));
}
