#include "testShutterFunction_sV.h"
#include "../project/shutterFunction_sV.h"
#include <cmath>

void TestShutterFunction_sV::init()
{
    int i = 0;
    app = new QCoreApplication(i, NULL);
}

void TestShutterFunction_sV::testZeroFunction()
{
    ShutterFunction_sV f("return 0;");
    for (float t = 0; t <= 1; t += .1) {
        QVERIFY(f.evaluate(t, 0, 0, 0) == 0);
    }
}

void TestShutterFunction_sV::testFunctions()
{
    ShutterFunction_sV f;

    f.updateFunction("return 1;");
    for (float t = 0; t <= 1; t += .1) {
        QVERIFY(f.evaluate(t, 0, 0, 0) == 1);
    }

    f.updateFunction("return Math.pow(x0, 2)+dt");
    for (float t = 0; t <= 1; t += .1) {
        float cpp = std::pow(t,2)+.5;
        float qsc = f.evaluate(t, .5, 0, 0);
//        qDebug() << "QScript says " << qsc << " (Qt: " << cpp << ")";
        QVERIFY(fabs(cpp - qsc) < .0001);
    }

    f.updateFunction("return dy*t0");
    for (float t = 0; t <= 1; t += .1) {
        float cpp = t*.1;
        float qsc = f.evaluate(0, 0, .1, t);
//        qDebug() << "QScript says " << qsc << " (Qt: " << cpp << ")";
        QVERIFY(fabs(cpp - qsc) < .0001);
    }
}
