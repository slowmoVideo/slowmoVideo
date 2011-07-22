#include "shutterFunctionFrame.h"

#include "lib/defs_sV.hpp"
#include "../canvas.h"
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

ShutterFunctionFrame::ShutterFunctionFrame(QWidget *parent, Qt::WindowFlags f) :
    QFrame(parent, f),
    m_frameHeight(100)
{
}
void ShutterFunctionFrame::updateValues(float dy, float t0)
{
    m_dy = dy;
    m_t0 = t0;
}

void ShutterFunctionFrame::paintEvent(QPaintEvent *e)
{
    qDebug() << "Repainting shutter curve";
    QFrame::paintEvent(e);

    int x, y;

    QPainter p(this);
    p.fillRect(rect(), Canvas::backgroundCol);

    y = m_frameHeight;
    while (y < height()) {
        p.setPen(Canvas::gridCol);
        if ((y % m_frameHeight) % 10 == 0) {
            p.setPen(Canvas::fatGridCol);
        }
        p.drawLine(0, height()-1 - y, width()-1, height()-1 - y);
        y += m_frameHeight;
    }


    p.setPen(Canvas::lineCol);

    float t;
    for (x = 0; x < width(); x++) {
        t = float(x)/width();
        y = height()-1 - m_frameHeight*m_function.evaluate(t, 1.0/24, m_dy, m_t0);
        p.drawPoint(x, y);
    }
}

void ShutterFunctionFrame::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0) {
        m_frameHeight *= 1.4;
    } else {
        m_frameHeight /= 1.4;
        if (m_frameHeight < 1) {
            m_frameHeight = 1;
        }
    }
    repaint();
}

void ShutterFunctionFrame::slotDisplayFunction(const QString &function)
{
    m_function.updateFunction(function);

    float max = qMax(m_function.evaluate(0, 1.0/24, m_dy, m_t0), qMax(
                         m_function.evaluate(.5, 1.0/24, m_dy, m_t0),
                         m_function.evaluate(1, 1.0/24, m_dy, m_t0)));
    if (max > 0) {
        while (m_frameHeight*max > height()) {
            m_frameHeight /= 1.4;
        }
        while (m_frameHeight*max < height()/5) {
            m_frameHeight *= 1.4;
        }
        while (m_frameHeight > height()) {
            m_frameHeight /= 1.4;
        }
    }

}
