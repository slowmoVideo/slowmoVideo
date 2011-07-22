#ifndef SHUTTERFUNCTIONFRAME_H
#define SHUTTERFUNCTIONFRAME_H

#include "project/shutterFunction_sV.h"
#include <QFrame>

class ShutterFunctionFrame : public QFrame
{
    Q_OBJECT
public:
    ShutterFunctionFrame(QWidget * parent = 0, Qt::WindowFlags f = 0);

    void updateValues(float dy, float t0);

public slots:
    void slotDisplayFunction(const QString &function);

protected slots:
    virtual void paintEvent(QPaintEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    ShutterFunction_sV m_function;
    int m_frameHeight;
    float m_dy;
    float m_t0;

};

#endif // SHUTTERFUNCTIONFRAME_H
