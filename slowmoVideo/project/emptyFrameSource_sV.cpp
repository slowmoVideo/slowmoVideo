#include "emptyFrameSource_sV.h"

EmptyFrameSource_sV::EmptyFrameSource_sV(const Project_sV *project) :
    AbstractFrameSource_sV(project)
{
}

void EmptyFrameSource_sV::initialize()
{
    emit signalAllTasksFinished();
}
