#ifndef TESTSHUTTERFUNCTION_SV_H
#define TESTSHUTTERFUNCTION_SV_H


#include <QObject>
#include <QtTest/QtTest>

class TestShutterFunction_sV : public QObject
{
    Q_OBJECT

private:
    QCoreApplication *app;

private slots:
    void init();
    void testZeroFunction();
    void testFunctions();
};

#endif // TESTSHUTTERFUNCTION_SV_H
