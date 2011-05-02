#include "opticalFlowBuilderGPUKLT_sV.h"

#include <QProcess>
#include <QDebug>

OpticalFlowBuilderGPUKLT_sV::OpticalFlowBuilderGPUKLT_sV()
{
}

OpticalFlowBuilderGPUKLT_sV::~OpticalFlowBuilderGPUKLT_sV()
{

}

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
