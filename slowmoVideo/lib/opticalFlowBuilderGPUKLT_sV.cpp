/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "opticalFlowBuilderGPUKLT_sV.h"

#include <QProcess>
#include <QDebug>

void OpticalFlowBuilderGPUKLT_sV::buildFlow(const QFile &left, const QFile &right, const QFile &output, FlowDirection direction) const
{
    QString program("/data/cworkspace/slowmoGPU/V3D/build/Apps/GL/tvl1_flow");
    QStringList args;
    switch (direction) {
    case FlowDirection_Forward:
        args << left.fileName() << right.fileName();
        break;
    case FlowDirection_Backward:
        args << right.fileName() << left.fileName();
        break;
    }
    args << "10" << "100" << output.fileName() << "exit";


    qDebug() << "Images: " << left.fileName() << ", " << right.fileName();

    QProcess *proc = new QProcess;
    proc->start(program, args);
    proc->waitForFinished();
    if (proc->exitCode() != 0) {
        qDebug() << "Failed: " << proc->readAllStandardError() << proc->readAllStandardOutput();
    } else {
        qDebug() << "Optical flow built for " << output.fileName();
        qDebug() << proc->readAllStandardError() << proc->readAllStandardOutput();
    }
}
