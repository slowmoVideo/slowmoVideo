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
