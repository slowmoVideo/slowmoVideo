#ifndef TESTSHUTTERFUNCTION_SV_H
#define TESTSHUTTERFUNCTION_SV_H


#include <QObject>
#include <QtTest/QtTest>

class TestShutterFunction_sV : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testZeroFunction();
    void testFunctions();

private:
    QCoreApplication *app;
};

#endif // TESTSHUTTERFUNCTION_SV_H
