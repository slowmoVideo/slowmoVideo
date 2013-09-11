#include "testDefs_sV.h"

#include "../lib/defs_sV.hpp"

void TestDefs_sV::testFpsInt()
{
    Fps_sV fps(24, 1);
    QVERIFY(fps.fps() == 24);
}

void TestDefs_sV::testFpsFloat()
{
    Fps_sV fps(24.0);
    QVERIFY(fps.num == 24);
    QVERIFY(fps.den == 1);

    fps = Fps_sV(23.97);
    QVERIFY(fps.num == 24000);
    QVERIFY(fps.den == 1001);

    fps = Fps_sV(29.97);
    QVERIFY(fps.num == 30000);
    QVERIFY(fps.den == 1001);
}
