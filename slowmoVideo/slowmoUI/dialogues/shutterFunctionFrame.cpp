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
void ShutterFunctionFrame::updateValues(float y, float dy)
{
    m_y = y;
    m_dy = dy;
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
        y = height()-1 - m_frameHeight*m_function.evaluate(t, t, 24, m_y, m_dy);
        p.drawPoint(x, y);
    }
}

void ShutterFunctionFrame::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0) {
        int old = m_frameHeight;
        m_frameHeight *= 1.4;
        if (m_frameHeight == old) {
            m_frameHeight++;
        }
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

    float max = qMax(m_function.evaluate(0, 0, 1.0/24, m_y, m_dy), qMax(
                         m_function.evaluate(.5, .5, 1.0/24, m_y, m_dy),
                         m_function.evaluate(1, 1, 1.0/24, m_y, m_dy)));
    if (max > 50) {
        max = 50;
    }
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
    repaint();
}
