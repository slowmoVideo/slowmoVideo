#include "testFlowField_sV.h"
#include "testFlowRW_sV.h"
#include "testIntMatrix_sV.h"
#include "testXmlProjectRW_sV.h"

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
}
