#include "testFlowField_sV.h"
#include "../lib/flowField_sV.h"

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
