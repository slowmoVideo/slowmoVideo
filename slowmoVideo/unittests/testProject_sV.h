#ifndef TESTPROJECT_SV_H
#define TESTPROJECT_SV_H

#include <QObject>
#include <QtTest/QtTest>

class Fps_sV;
class Project_sV;

class TestProject_sV : public QObject
{
    Q_OBJECT
private slots:
    void slotTestSnapInFrames();
    void slotTestTimeExpressions();
    void slotTestPercentageExpressions();
    void slotTestLabelExpressions();
    void slotTsetPositionExpressions();
    void init();
    void cleanup();

private:
    Project_sV *m_project;
    Fps_sV *m_fps;
};

#endif // TESTPROJECT_SV_H
