#include "testShutterFunction_sV.h"
#include "../project/shutterFunction_sV.h"
#include <cmath>

void TestShutterFunction_sV::initTestCase()
{
    int i = 0;
    app = new QCoreApplication(i, NULL);
}
void TestShutterFunction_sV::cleanupTestCase()
{
    delete app;
}

void TestShutterFunction_sV::testZeroFunction()
{
    ShutterFunction_sV f("return 0;");
    for (float t = 0; t <= 1; t += .1) {
        QVERIFY(f.evaluate(t, 0, 0, 0, 0) == 0);
    }
}

void TestShutterFunction_sV::testFunctions()
{
    ShutterFunction_sV f;

    f.updateFunction("return 1;");
    for (float t = 0; t <= 1; t += .1) {
        QVERIFY(f.evaluate(t, 0, 0, 0, 0) == 1);
    }

    f.updateFunction("return Math.pow(x, 2)+t");
    for (float t = 0; t <= 1; t += .1) {
        float cpp = std::pow(t,2)+t;
        float qsc = f.evaluate(t, t, 0, 0, 0);
//        qDebug() << "QScript says " << qsc << " (Qt: " << cpp << ")";
        QVERIFY(fabs(cpp - qsc) < .0001);
    }

    f.updateFunction("return fps+y*dy");
    for (float t = 0; t <= 1; t += .1) {
        float cpp = 24+t;
        float qsc = f.evaluate(0, 0, 24, t, 1);
//        qDebug() << "QScript says " << qsc << " (Qt: " << cpp << ")";
        QVERIFY(fabs(cpp - qsc) < .0001);
    }
}
