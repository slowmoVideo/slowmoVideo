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

void TestProject_sV::init()
{
    m_project = new Project_sV();
    m_project->nodes()->add(Node_sV(1,42));
    m_project->nodes()->add(Node_sV(5,21));
    m_project->tags()->append(Tag_sV(4.5, "sourceLabel", TagAxis_Source));
    m_project->tags()->append(Tag_sV(2.5, "outputLabel", TagAxis_Output));
    m_fps = new Fps_sV(10, 1);
}
void TestProject_sV::cleanup()
{
    delete m_project;
    delete m_fps;
}

void TestProject_sV::slotTestTimeExpressions()
{
    Project_sV project;
    project.nodes()->add(Node_sV(1, 42));
    Fps_sV fps(10, 1);
    try {
        project.toOutTime(0, fps);
        QVERIFY(false);
    } catch (Error_sV &err) {}

    project.nodes()->add(Node_sV(5, 21));
    QVERIFY(qreal(1) == project.toOutTime("1", fps));
    QVERIFY(qreal(1) == project.toOutTime("-1", fps));
    QVERIFY(qreal(5) == project.toOutTime("55", fps));
    QVERIFY(qreal(4.5) == project.toOutTime("4.5", fps));
}

void TestProject_sV::slotTestPercentageExpressions()
{
    QVERIFY(qreal(3) == m_project->toOutTime("p:50%", *m_fps));
    try {
        m_project->toOutTime("p:101%", *m_fps);
        QVERIFY(false);
    } catch (Error_sV &err) {}
}

void TestProject_sV::slotTestLabelExpressions()
{
    QVERIFY(qreal(2.5) == m_project->toOutTime("l:outputLabel", *m_fps));
    try {
        m_project->toOutTime("l:sourceLabel", *m_fps);
        QVERIFY(false);
    } catch (Error_sV &err) {}
}

void TestProject_sV::slotTsetPositionExpressions()
{
    QVERIFY(m_project->nodes()->startTime() == m_project->toOutTime(":start", *m_fps));
    QVERIFY(m_project->nodes()->endTime() == m_project->toOutTime(":end", *m_fps));
}
