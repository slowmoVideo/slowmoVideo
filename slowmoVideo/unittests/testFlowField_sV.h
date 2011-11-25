#ifndef TESTFLOWFIELD_SV_H
#define TESTFLOWFIELD_SV_H

#include <QObject>
#include <QtTest/QtTest>

class TestFlowField_sV : public QObject
{
    Q_OBJECT
private slots:
    void slotTestConstructorOpenGL();
    void slotTestGaussKernel();
};

#endif // TESTFLOWFIELD_SV_H
