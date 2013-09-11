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

void TestShutterFunction_sV::testWithVariables()
{
    ShutterFunction_sV f;
    f.updateFunction("var dx = 1/fps; \n"
                     "var speed = dy/dx;\n"
                     "if (speed < 1) { speed = 0; }\n"
                     "return speed;");
    float dx = 1.0/24;
    for (float dy = 0; dy <= 1; dy += .1) {
        float speed = dy/dx;
        if (speed < 1) { speed = 0; }
        float qsc = f.evaluate(0, 0, 24, 0, dy);
//        qDebug() << "QScript says " << qsc << " (Qt: " << speed << ")";
        QVERIFY(fabs(speed-qsc) < .0001);
    }
}
