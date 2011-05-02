#include "flow_sV.h"

#include <QFile>
#include <QDebug>

#include "../lib/opticalFlowBuilder_sV.h"
#include "../lib/opticalFlowBuilderGPUKLT_sV.h"

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

Flow_sV::Flow_sV() {}
Flow_sV::~Flow_sV() {}

void Flow_sV::buildFlow(Project_sV *project, ProjectFrameMemFn frameNames, ProjectFlowMemFn outName, FlowDirection direction) const
{
    int framesCount = project->videoInfo().framesCount;

    OpticalFlowBuilder_sV *builder = new OpticalFlowBuilderGPUKLT_sV();
    for (int i = 2; i < framesCount; i++) {
        QString left, right, forward;
        left = CALL_MEMBER_FN(project, frameNames)(i-1);
        right = CALL_MEMBER_FN(project, frameNames)(i);
        forward = CALL_MEMBER_FN(project, outName)(i, direction);
        if (!QFile(forward).exists()) {
            builder->buildFlow(
                        left, right, forward,
                        FlowDirection_Forward
                        );
        } else {
            qDebug() << forward << " already exists.";
        }

    }
    delete builder;
}
