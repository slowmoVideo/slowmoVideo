#include "testFlowField_sV.h"
#include "testFlowRW_sV.h"
#include "testIntMatrix_sV.h"
#include "testXmlProjectRW_sV.h"
#include "testVector_sV.h"
#include "testDefs_sV.h"
#include "testShutterFunction_sV.h"

#include <QtTest/QtTest>

int main()
{
    TestFlowRW_sV flowRW;
    QTest::qExec(&flowRW);

    TestFlowField_sV flowField;
    QTest::qExec(&flowField);

    TestIntMatrix_sV intMatrix;
    QTest::qExec(&intMatrix);

    TestXmlProjectRW_sV xmlRW;
    QTest::qExec(&xmlRW);

    TestVector_sV vector;
    QTest::qExec(&vector);

    TestDefs_sV defs;
    QTest::qExec(&defs);

    TestShutterFunction_sV shutter;
    QTest::qExec(&shutter);
}
