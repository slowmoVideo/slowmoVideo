#include "testFlowField_sV.h"
#include "testFlowRW_sV.h"

#include <QtTest/QtTest>

int main()
{
    TestFlowRW_sV flowRW;
    QTest::qExec(&flowRW);

    TestFlowField_sV flowField;
    QTest::qExec(&flowField);
}
