#include "testProject_sV.h"

#include "../project/project_sV.h"
#include <QtCore/QDebug>

void TestProject_sV::slotTestSnapInFrames()
{
    int framesBefore;
    float pos;
    Fps_sV fps(10, 1);

    pos = Project_sV::snapToFrame(0, false, fps, &framesBefore);
    QVERIFY(pos == 0);
    QVERIFY(framesBefore == 0);

    pos = Project_sV::snapToFrame(0, true, fps, &framesBefore);
    QVERIFY(pos == 0);
    QVERIFY(framesBefore == 0);

    pos = Project_sV::snapToFrame(0.49, false, fps, &framesBefore);
    QVERIFY(pos == (float).4);
    QVERIFY(framesBefore == 4);

    pos = Project_sV::snapToFrame(0.49, true, fps, &framesBefore);
    QVERIFY(pos == (float).5);
    QVERIFY(framesBefore == 5);
}
