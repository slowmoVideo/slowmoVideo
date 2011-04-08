/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "canvas.h"
#include "node.h"
#include "nodelist.h"
#include "ui_canvas.h"

#include <cmath>

#include <QDebug>

#include <QColor>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>

#include <QPoint>
#include <QPointF>

#define NODE_RADIUS 4
#define NODE_SEL_DIST 2

QColor Canvas::selectedCol(200, 200, 255);
QColor Canvas::lineCol(220, 220, 220);
QColor Canvas::nodeCol(240, 240, 240);
QColor Canvas::gridCol(100, 100, 100);
QColor Canvas::backgroundCol(30, 30, 40);

Canvas::Canvas(QWidget *parent) :
    QWidget(parent),
    m_frameRate(30.0f),
    ui(new Ui::Canvas),
    m_lastMousePos(0,0),
    m_mouseStart(0,0),
    m_mouseWithinWidget(false),
    m_distLeft(50),
    m_distBottom(50),
    m_distRight(20),
    m_distTop(32),
    m_t0x(0),
    m_t0y(0),
    m_secResX(100),
    m_secResY(100),
    m_nodes(),
    m_moveAborted(false),
    m_mode(ToolMode_Add)
{
    ui->setupUi(this);

    // Enable mouse tracking (not only when a mouse button is pressed)
    this->setMouseTracking(true);
}

Canvas::~Canvas()
{
    delete ui;
}


bool Canvas::selectAt(const QPoint &pos, bool addToSelection)
{
    bool selected = false;
    uint ti = m_nodes.find(convertCanvasToTime(pos).x());
    qDebug() << "Nearest node index: " << ti;
    if (m_nodes.size() > ti) {
        QPoint p = convertTimeToCanvas(m_nodes.at(ti));
        if (
                abs(p.x() - pos.x()) <= NODE_RADIUS+1 &&
                abs(p.y() - pos.y()) <= NODE_RADIUS+1
            ) {
            qDebug() << "Selected.";


            if (!m_nodes.at(ti).selected() && !addToSelection) {
                m_nodes.unselectAll();
            }

            if (addToSelection) {
                m_nodes[ti].select(!m_nodes.at(ti).selected());
            } else {
                m_nodes[ti].select(true);
            }
            selected = true;
        }
    }
    return selected;
}

bool Canvas::insideCanvas(const QPoint &pos)
{
    return  pos.x() >= m_distLeft &&
            pos.y() >= m_distTop &&
            pos.x() < width()-m_distRight &&
            pos.y() < height()-m_distBottom;
}



void Canvas::paintEvent(QPaintEvent *)
{
    QPainter davinci(this);
    davinci.setRenderHint(QPainter::Antialiasing, true);

    davinci.fillRect(0, 0, width(), height(), backgroundCol);

    davinci.setPen(gridCol);
    for (int tx = ceil(m_t0x); true; tx++) {
        QPoint pos = convertTimeToCanvas(Node(tx, m_t0y));
        if (insideCanvas(pos)) {
            davinci.drawLine(pos.x(), pos.y(), pos.x(), m_distTop);
        } else {
//            qDebug() << "Out of canvas: " << pos;
            break;
        }
    }
    for (int ty = ceil(m_t0y); true; ty++) {
        QPoint pos = convertTimeToCanvas(Node(m_t0x, ty));
        if (insideCanvas(pos)) {
            davinci.drawLine(pos.x(), pos.y(), width()-1 - m_distRight, pos.y());
        } else {
//            qDebug() << "Out of canvas: " << pos;
            break;
        }
    }

    davinci.setOpacity(.5 + ((m_mode == ToolMode_Add) ? .5 : 0));
    davinci.drawImage(width()-m_distRight-16, 8, QImage("res/iconAdd.png").scaled(16, 16));
    davinci.setOpacity(.5 + ((m_mode == ToolMode_Select) ? .5 : 0));
    davinci.drawImage(width()-m_distRight-16-24, 8, QImage("res/iconSel.png").scaled(16, 16));
    davinci.setOpacity(1);

    davinci.setPen(lineCol);
    if (m_mouseWithinWidget && insideCanvas(m_lastMousePos)) {
        davinci.drawLine(m_lastMousePos.x(), m_distTop, m_lastMousePos.x(), height()-1 - m_distBottom);
        Node time = convertCanvasToTime(m_lastMousePos);
        davinci.drawText(m_lastMousePos.x() - 20, height()-1 - 20, QString("%1 s").arg(time.x()));
        davinci.drawLine(m_distLeft, m_lastMousePos.y(), m_lastMousePos.x(), m_lastMousePos.y());
        davinci.drawText(8, m_lastMousePos.y(), QString("f %1").arg(time.y()/m_frameRate));
    }
    int bottom = height()-1 - m_distBottom;
    davinci.drawLine(m_distLeft, bottom, width()-1 - m_distRight, bottom);
    davinci.drawLine(m_distLeft, bottom, m_distLeft, m_distTop);

    const Node *prev = NULL;
    const Node *curr = NULL;
    for (uint i = 0; i < m_nodes.size(); i++) {
        curr = &m_nodes.at(i);

        QPoint p = convertTimeToCanvas(*curr);

        davinci.setPen(nodeCol);
        davinci.drawRect(p.x()-NODE_RADIUS, p.y()-NODE_RADIUS, 2*NODE_RADIUS+1, 2*NODE_RADIUS+1);
        if (curr->selected()) {
            davinci.setPen(selectedCol);
            davinci.drawRect(p.x()-NODE_RADIUS-NODE_SEL_DIST, p.y()-NODE_RADIUS-NODE_SEL_DIST, 2*(NODE_RADIUS+NODE_SEL_DIST)+1, 2*(NODE_RADIUS+NODE_SEL_DIST)+1);
        }
        if (prev != NULL) {
            davinci.setPen(lineCol);
            davinci.drawLine(convertTimeToCanvas(*prev), p);
        }

        prev = &m_nodes.at(i);
    }
}

void Canvas::mousePressEvent(QMouseEvent *e)
{
    m_moveAborted = false;
    if (e->pos().x() >= m_distLeft && e->pos().y() < this->height()-m_distBottom) {
        // Try to select a node below the mouse. If there is none, add a point.
        bool selected = selectAt(e->pos(), e->modifiers() && Qt::ControlModifier);
        if (!selected) {
            if (m_mode == ToolMode_Add) {
                Node p = convertCanvasToTime(e->pos());
                m_nodes.add(p);
            } else {
                qDebug() << "Not adding node. Mode is " << m_mode;
            }
        } else {
            qDebug() << "Node selected.";
        }
        repaint();

        qDebug() << "Node list: " << m_nodes;
    } else {
        qDebug() << "Not inside bounds.";
    }
    m_mouseStart = e->pos();
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    m_lastMousePos = e->pos();
    m_mouseWithinWidget = true;

    if (e->buttons() && Qt::LeftButton != 0) {
        if (m_mode == ToolMode_Select) {
            if (!m_moveAborted) {
                qDebug() << m_mouseStart << "to" << e->pos();
                Node diff = convertCanvasToTime(e->pos()) - convertCanvasToTime(m_mouseStart);
                qDebug() << "Diff: " << diff;
                if (qAbs(diff.x()) < qAbs(diff.y())) {
                    diff.setX(0);
                } else {
                    diff.setY(0);
                }
                m_nodes.moveSelected(diff);
            }
        }
    }

    repaint();
}

void Canvas::mouseReleaseEvent(QMouseEvent *)
{
    if (m_mode == ToolMode_Select && !m_moveAborted) {
        m_nodes.confirmMove();
    }
}

void Canvas::leaveEvent(QEvent *)
{
    m_mouseWithinWidget = false;
    repaint();
}

const Node Canvas::convertCanvasToTime(const QPoint &p) const
{
    Node out(
                m_t0x + float(p.x()-m_distLeft)/m_secResX,
                m_t0y + float(this->height()-1 - m_distBottom - p.y()) / m_secResY
            );
    return out;
}

const QPoint Canvas::convertTimeToCanvas(const Node &p) const
{
    QPoint out(
                (p.x()-m_t0x)*m_secResX + m_distLeft,
                this->height()-1 - m_distBottom - (p.y()-m_t0y)*m_secResY
           );
    return out;
}


void Canvas::slotAbort(Canvas::Abort abort)
{
    qDebug() << "Signal: " << abort;
    switch (abort) {
    case Abort_General:
        m_moveAborted = true;
        m_nodes.abortMove();
        repaint();
        break;
    case Abort_Selection:
        m_nodes.unselectAll();
        repaint();
        break;
    }

}

void Canvas::slotDeleteNodes()
{
    qDebug() << "Will delete";
    uint nDel = m_nodes.deleteSelected();
    qDebug() << nDel << " deleted.";
    if (nDel > 0) {
        repaint();
    }
}

void Canvas::slotSetToolMode(ToolMode mode)
{
    m_mode = mode;
    qDebug() << "Mode set to: " << mode;
    repaint();
}

QDebug operator <<(QDebug qd, const Canvas::ToolMode &mode)
{
    switch(mode) {
    case Canvas::ToolMode_Add:
        qd << "Add tool";
        break;
    case Canvas::ToolMode_Select:
        qd << "Select tool";
        break;
    }
    return qd.maybeSpace();
}

QDebug operator <<(QDebug qd, const Canvas::Abort &abort)
{
    switch(abort) {
    case Canvas::Abort_General:
        qd << "Abort General";
        break;
    case Canvas::Abort_Selection:
        qd << "Abort Selection";
        break;
    }
    return qd.maybeSpace();
}
