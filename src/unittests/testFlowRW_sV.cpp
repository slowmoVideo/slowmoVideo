#include "testFlowRW_sV.h"

#include <QDebug>
#include <string>
#include <flowField_sV.h>
#include <flowRW_sV.h>

void TestFlowRW_sV::testWriteAndRead()
{
    const std::string filename("/tmp/unittestFlowField_sV.sVflow");

    const int width = 3;
    const int height = 2;
    FlowField_sV *field = new FlowField_sV(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            field->setX(x, y, x/float(y+1));
            field->setY(x, y, x/float(y+2));
        }
    }
    FlowRW_sV::save(filename, field);

    FlowField_sV *loadedField = FlowRW_sV::load(filename);

    QVERIFY(*field == *loadedField);

    delete field;
    delete loadedField;
}
void TestFlowRW_sV::testWriteAndReadFail()
{
    const std::string filename("/tmp/unittestFlowField_sV.sVflow");

    const int width = 3;
    const int height = 2;
    FlowField_sV *field = new FlowField_sV(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            field->setX(x, y, x/float(y+1));
            field->setY(x, y, x/float(y+2));
        }
    }
    FlowRW_sV::save(filename, field);



    field->setX(width-1, height-1, -3.1415927);
    FlowField_sV *loadedField = FlowRW_sV::load(filename);

    qDebug() << "Equal? " << (field->x(width-1, height-1) == loadedField->x(width-1, height-1));

    QVERIFY( !( *field == *loadedField ) );

    delete field;
    delete loadedField;
}

