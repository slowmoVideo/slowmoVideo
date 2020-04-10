#ifndef TESTFLOWFIELD_SV_H
#define TESTFLOWFIELD_SV_H

#include <QObject>
#include <QtTest/QtTest>
class FlowField_sV;

class TestFlowField_sV : public QObject
{
    Q_OBJECT
private slots:
    void slotTestConstructorOpenGL();
    void slotTestGaussKernel();
    void slotTestMedian();
private:
    void initFlowField(FlowField_sV *field, int *values);
};

#endif // TESTFLOWFIELD_SV_H
