#ifndef TESTDEFS_SV_H
#define TESTDEFS_SV_H

#include <QObject>
#include <QtTest/QtTest>

class TestDefs_sV : public QObject
{
    Q_OBJECT

private slots:
    void testFpsInt();
    void testFpsFloat();
};

#endif // TESTDEFS_SV_H
