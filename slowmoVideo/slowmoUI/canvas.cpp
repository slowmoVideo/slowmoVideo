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

#include <QDebug>

#include <QColor>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>

#include <QPoint>
#include <QPointF>

#define NODE_RADIUS 4

QColor Canvas::selectedCol(200, 200, 255);
QColor Canvas::lineCol(220, 220, 220);
QColor Canvas::nodeCol(240, 240, 240);
QColor Canvas::backgroundCol(30, 30, 40);

Canvas::Canvas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Canvas),
    m_lastMousePos(0,0),
    m_mouseWithinWidget(false),
    m_distLeft(50),
    m_distBottom(50),
    m_t0x(0),
    m_t0y(0),
    m_secResX(100),
    m_secResY(100),
    m_nodes()
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
                abs(p.x() - pos.x()) <= NODE_RADIUS &&
                abs(p.y() - pos.y()) <= NODE_RADIUS
            ) {
            qDebug() << "Selected.";

            if (!addToSelection) {
                m_nodes.unselectAll();
            }
            m_nodes[ti].select(true);
            selected = true;
        }
    }
    return selected;
}



void Canvas::paintEvent(QPaintEvent *)
{
    QPainter davinci(this);
    davinci.setRenderHint(QPainter::Antialiasing, true);

    davinci.fillRect(0, 0, width(), height(), backgroundCol);

    davinci.setPen(lineCol);
    if (m_mouseWithinWidget) {
        davinci.drawLine(m_lastMousePos.x(), 0, m_lastMousePos.x(), this->height()-1);
    }
    int bottom = height()-1 - m_distBottom;
    davinci.drawLine(m_distLeft, bottom, width()-1, bottom);
    davinci.drawLine(m_distLeft, bottom, m_distLeft, 0);

    const Node *prev = NULL;
    const Node *curr = NULL;
    for (int i = 0; i < m_nodes.size(); i++) {
        curr = &m_nodes.at(i);

        QPoint p = convertTimeToCanvas(*curr);

        davinci.setPen(nodeCol);
        davinci.drawRect(p.x()-NODE_RADIUS, p.y()-NODE_RADIUS, 2*NODE_RADIUS+1, 2*NODE_RADIUS+1);
        if (curr->selected()) {
            davinci.setPen(selectedCol);
            davinci.drawRect(p.x()-NODE_RADIUS-1, p.y()-NODE_RADIUS-1, 2*NODE_RADIUS+3, 2*NODE_RADIUS+3);
        }
        if (prev != NULL) {
            davinci.setPen(lineCol);
            davinci.drawLine(convertTimeToCanvas(*prev), p);
        }

        prev = &m_nodes.at(i);
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    m_lastMousePos = e->pos();
    m_mouseWithinWidget = true;
    repaint();
}

void Canvas::mousePressEvent(QMouseEvent *e)
{
    if (e->pos().x() >= m_distLeft && e->pos().y() < this->height()-m_distBottom) {
        // Try to select a node below the mouse. If there is none, add a point.
        bool selected = selectAt(e->pos(), e->modifiers() && Qt::ControlModifier);
        if (!selected) {
            Node p = convertCanvasToTime(e->pos());
            convertTimeToCanvas(p);
            m_nodes.add(p);
        }
        repaint();
        qDebug() << "Node list: " << m_nodes;
    } else {
        qDebug() << "Not inside bounds.";
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


void Canvas::slotDeleteNodes()
{
    qDebug() << "Will delete";
    uint nDel = m_nodes.deleteSelected();
    qDebug() << nDel << " deleted.";
    if (nDel > 0) {
        repaint();
    }
}
