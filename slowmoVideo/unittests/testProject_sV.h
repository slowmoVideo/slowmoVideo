#ifndef TESTPROJECT_SV_H
#define TESTPROJECT_SV_H

#include <QObject>
#include <QtTest/QtTest>

class TestProject_sV : public QObject
{
    Q_OBJECT
private slots:
    void slotTestSnapInFrames();
};

#endif // TESTPROJECT_SV_H
