#include "flow_sV.h"

#include <QFile>
#include <QDebug>

#include "../lib/opticalFlowBuilder_sV.h"
#include "../lib/opticalFlowBuilderGPUKLT_sV.h"

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

void Flow_sV::buildFlow(Project_sV *project, ProjectFrameMemFn frameNames, ProjectFlowMemFn outName, FlowDirection direction)
{
    abort = false;
    aborted = false;
    int framesCount = project->videoInfo().framesCount;

    OpticalFlowBuilder_sV *builder = new OpticalFlowBuilderGPUKLT_sV();
    for (int i = 2; i < framesCount; i++) {
        if (abort) {
            aborted = true;
            break;
        }
        QString left, right, forward;
        left = CALL_MEMBER_FN(project, frameNames)(i-1);
        right = CALL_MEMBER_FN(project, frameNames)(i);
        forward = CALL_MEMBER_FN(project, outName)(i, direction);

        emit signalFlowFrame(forward);

        if (!QFile(forward).exists()) {
            builder->buildFlow(
                        left, right, forward,
                        FlowDirection_Forward
                        );
        } else {
            qDebug() << forward << " already exists.";
        }
        emit signalFlowProgressUpdated(i);

    }
    if (aborted) {
        emit signalFlowAborted();
    } else {
        emit signalFlowFinished();
    }
    delete builder;
}

void Flow_sV::slotAbort()
{
    abort = true;
}
