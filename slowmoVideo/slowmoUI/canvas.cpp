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



void Canvas::paintEvent(QPaintEvent *)
{
    QPainter davinci(this);
    QImage im(this->size(), QImage::Format_ARGB32);
    im.fill(QColor(50, 50, 60).rgb());
    davinci.drawImage(0, 0, im);
    davinci.setPen(QColor::fromRgb(240, 240, 240));
    if (m_mouseWithinWidget) {
        qDebug() << "lining.";
        davinci.drawLine(m_lastMousePos.x(), 0, m_lastMousePos.x(), this->height()-1);
    }

    const Node *prev = NULL;
    for (int i = 0; i < m_nodes.size(); i++) {
        davinci.drawEllipse(convertTimeToCanvas(m_nodes.at(i)), 3, 3);
        if (prev != NULL) {
            davinci.drawLine(convertTimeToCanvas(*prev), convertTimeToCanvas(m_nodes.at(i)));
        }
        prev = &m_nodes.at(i);
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    m_lastMousePos = e->pos();
    m_mouseWithinWidget = true;
    qDebug() << "At " << m_lastMousePos;
    this->repaint();
}

void Canvas::mousePressEvent(QMouseEvent *e)
{
    if (e->pos().x() >= m_distLeft && e->pos().y() < this->height()-m_distBottom) {
        Node p = convertCanvasToTime(e->pos());
        convertTimeToCanvas(p);
        m_nodes.add(p);
        this->repaint();
    } else {
        qDebug() << "Not inside bounds.";
    }
}

void Canvas::leaveEvent(QEvent *)
{
    m_mouseWithinWidget = false;
}

const Node Canvas::convertCanvasToTime(const QPoint &p) const
{
    Node out(
                m_t0x + float(p.x()-m_distLeft)/m_secResX,
                m_t0y + float(this->height()-1 - m_distBottom - p.y()) / m_secResY
            );

    qDebug() << "Time: " << out.x() << "|" << out.y();

    return out;
}

const QPoint Canvas::convertTimeToCanvas(const Node &p) const
{
    QPoint out(
                (p.x()-m_t0x)*m_secResX + m_distLeft,
                this->height()-1 - m_distBottom - (p.y()-m_t0y)*m_secResY
           );
    qDebug() << "Point: " << out;
    return out;
}
