#ifndef TESTFLOWRW_SV_H
#define TESTFLOWRW_SV_H

#include <QObject>
#include <QtTest/QtTest>

class TestFlowRW_sV : public QObject
{
    Q_OBJECT

private slots:
    void testWriteAndRead();
    void testWriteAndReadFail();

};

#endif // TESTFLOWRW_SV_H
