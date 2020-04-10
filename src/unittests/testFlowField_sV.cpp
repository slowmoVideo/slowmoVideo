#include "testFlowField_sV.h"
#include "../lib/flowField_sV.h"
#include "../lib/flowTools_sV.h"

#include <iostream>

void TestFlowField_sV::slotTestConstructorOpenGL()
{
    const int width = 2;
    const int height = 2;

    FlowField_sV field(width, height);
    for (int i = 0; i < 2*width*height; i++) {
        field.data()[i] = i;
    }

    float *glData = new float[3*width*height];
    float *pData = glData;
    for (int i = 0; i < width*height; i++) {
        *(pData++) = field.data()[2*i+0];
        *(pData++) = field.data()[2*i+1];
        pData++;
    }

    FlowField_sV *glField = new FlowField_sV(width, height, glData, FlowField_sV::GLFormat_RGB);

    QVERIFY(field == *glField);

    delete[] glData;
    delete glField;
}

void TestFlowField_sV::slotTestGaussKernel()
{
    Kernel_sV kernel(2,2);
    kernel.gauss();
    std::cout << kernel;
    QVERIFY(fabs(kernel(0,0)-1) < .001);
    QVERIFY(fabs(kernel(-1,-1)-.135) < .001);
    QVERIFY(kernel(-1,-1) == kernel(1,1));

    kernel = Kernel_sV(3,1);
    kernel.gauss();
    std::cout << kernel;
    QVERIFY(fabs(kernel(0,0)-1) < .001);
    QVERIFY(fabs(kernel(-3,-1)) < .001);

}

void TestFlowField_sV::slotTestMedian()
{
    int *values = new int[4];

    values[0] = 0; values[1] = 0;
    values[2] = 0; values[3] = 2;
    FlowField_sV f1(2,2);
    initFlowField(&f1, values);

    values[0] = 0; values[1] = 1;
    values[2] = 0; values[3] = 1;
    FlowField_sV f2(2,2);
    initFlowField(&f2, values);

    values[0] = 0; values[1] = 2;
    values[2] = 1; values[3] = 0;
    FlowField_sV f3(2,2);
    initFlowField(&f3, values);

    FlowField_sV *outField = FlowTools_sV::median(&f1, &f2, &f3);

    values[0] = 0; values[1] = 1;
    values[2] = 0; values[3] = 1;

    QVERIFY(outField->x(0,0) == values[0]);
    QVERIFY(outField->x(1,0) == values[1]);
    QVERIFY(outField->x(0,1) == values[2]);
    QVERIFY(outField->x(1,1) == values[3]);
    QVERIFY(outField->y(0,0) == values[0]);
    QVERIFY(outField->y(1,0) == values[1]);
    QVERIFY(outField->y(0,1) == values[2]);
    QVERIFY(outField->y(1,1) == values[3]);

    delete values;
    delete outField;
}

void TestFlowField_sV::initFlowField(FlowField_sV *field, int *values)
{
    int c = 0;
    for (int y = 0; y < field->height(); y++) {
        for (int x = 0; x < field->width(); x++) {
            field->rx(x,y) = values[c];
            field->ry(x,y) = values[c];
            c++;
        }
    }
}
