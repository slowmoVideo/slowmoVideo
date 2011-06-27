/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "canvas.h"
#include "ui_canvas.h"

#include "mainwindow.h"
#include "tagAddDialog.h"

#include "../project/abstractFrameSource_sV.h"

#include <cmath>

#include <QDebug>

#include <QColor>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>

#include <QPoint>
#include <QPointF>

QColor Canvas::selectedCol(255, 196, 0);
QColor Canvas::lineCol(220, 220, 220);
QColor Canvas::nodeCol(240, 240, 240);
QColor Canvas::gridCol(100, 100, 100);
QColor Canvas::labelCol(0, 77, 255);
QColor Canvas::backgroundCol(30, 30, 40);

/// \todo zoom in/out, scrolling etc.: scaling
/// \todo move with MMB

Canvas::Canvas(Project_sV *project, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Canvas),
    m_project(project),
    m_mouseWithinWidget(false),
    m_distLeft(90),
    m_distBottom(50),
    m_distRight(20),
    m_distTop(32),
    m_t0(0,0),
    m_tmax(10,10),
    m_secResX(100),
    m_secResY(100),
    m_showHelp(false),
    m_nodes(project->nodes()),
    m_tags(project->tags()),
    m_mode(ToolMode_Select)
{
    ui->setupUi(this);

    // Enable mouse tracking (not only when a mouse button is pressed)
    this->setMouseTracking(true);

    m_states.prevMousePos = QPoint(0,0);

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);
}

Canvas::~Canvas()
{
    delete ui;
}

void Canvas::load(Project_sV *project)
{
    m_project = project;
    qDebug() << "Canvas: Project loaded from " << project;
    m_nodes = project->nodes();
    m_tags = project->tags();
    qDebug() << "Frame source: " << project->frameSource();
    m_tmax.setY(project->frameSource()->framesCount() / project->frameSource()->fps());
    qDebug() << "tMaxY set to " << m_tmax.y();
    repaint();
}

void Canvas::toggleHelp()
{
    m_showHelp = !m_showHelp;
    repaint();
}


bool Canvas::selectAt(const QPoint &pos, bool addToSelection)
{
    bool selected = false;
    int ti = m_nodes->find(convertCanvasToTime(pos).x());
    qDebug() << "Nearest node index: " << ti;
    if (ti != -1 && m_nodes->size() > ti) {
        QPoint p = convertTimeToCanvas(m_nodes->at(ti));
        qDebug() << "Mouse pos: " << pos << ", node pos: " << p;
        if (
                abs(p.x() - pos.x()) <= NODE_RADIUS+NODE_SEL_DIST+4 &&
                abs(p.y() - pos.y()) <= NODE_RADIUS+NODE_SEL_DIST+4
            ) {
            qDebug() << "Selected: " << pos.x() << "/" << pos.y();


            if (!m_nodes->at(ti).selected() && !addToSelection) {
                m_nodes->unselectAll();
            }

            if (addToSelection) {
                (*m_nodes)[ti].select(!m_nodes->at(ti).selected());
            } else {
                (*m_nodes)[ti].select(true);
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
    // x grid
    for (int tx = ceil(m_t0.x()); true; tx++) {
        QPoint pos = convertTimeToCanvas(Node_sV(tx, m_t0.y()));
        if (insideCanvas(pos)) {
            davinci.drawLine(pos.x(), pos.y(), pos.x(), m_distTop);
        } else {
            break;
        }
    }
    // y grid
    for (int ty = ceil(m_t0.y()); true; ty++) {
        QPoint pos = convertTimeToCanvas(Node_sV(m_t0.x(), ty));
        if (insideCanvas(pos)) {
            davinci.drawLine(pos.x(), pos.y(), width()-1 - m_distRight, pos.y());
        } else {
            break;
        }
    }
    {
        QPoint pos = convertTimeToCanvas(Node_sV(m_t0.x(), m_tmax.y()));
        if (insideCanvas(pos)) {
            davinci.setPen(QPen(QBrush(lineCol), 2));
            davinci.drawLine(pos.x(), pos.y(), width()-1-m_distRight, pos.y());
        }
    }

    drawModes(davinci, 8, width()-1 - m_distRight);

    // Frames/seconds
    davinci.setPen(lineCol);
    if (m_mouseWithinWidget && insideCanvas(m_states.prevMousePos)) {
        davinci.drawLine(m_states.prevMousePos.x(), m_distTop, m_states.prevMousePos.x(), height()-1 - m_distBottom);
        Node_sV time = convertCanvasToTime(m_states.prevMousePos);
        davinci.drawText(m_states.prevMousePos.x() - 20, height()-1 - 20, QString("%1 s").arg(time.x()));
        davinci.drawLine(m_distLeft, m_states.prevMousePos.y(), m_states.prevMousePos.x(), m_states.prevMousePos.y());
        davinci.drawText(8, m_states.prevMousePos.y()-6, m_distLeft-2*8, 20, Qt::AlignRight,
                         QString("f %1").arg(time.y()*m_project->frameSource()->fps(), 2, 'f', 2));
    }
    int bottom = height()-1 - m_distBottom;
    davinci.drawLine(m_distLeft, bottom, width()-1 - m_distRight, bottom);
    davinci.drawLine(m_distLeft, bottom, m_distLeft, m_distTop);

    // Tags
    davinci.setPen(labelCol);
    for (int i = 0; i < m_tags->size(); i++) {
        Tag_sV tag = m_tags->at(i);
        QPoint p = convertTimeToCanvas(Node_sV(m_t0.x(), tag.time()));
        if (insideCanvas(p)) {
            davinci.drawLine(m_distLeft, p.y(), width()-m_distRight, p.y());
            davinci.drawText(m_distLeft+10, p.y()-1, tag.description());
        }
    }

    // Nodes
    davinci.setPen(lineCol);
    const Node_sV *prev = NULL;
    const Node_sV *curr = NULL;
    for (int i = 0; i < m_nodes->size(); i++) {
        curr = &m_nodes->at(i);

        QPoint p = convertTimeToCanvas(*curr);

        davinci.setPen(nodeCol);
        davinci.drawRect(p.x()-NODE_RADIUS, p.y()-NODE_RADIUS, 2*NODE_RADIUS+1, 2*NODE_RADIUS+1);
        if (curr->selected()) {
            davinci.setPen(QPen(QBrush(selectedCol), 2.0));
            davinci.drawRoundedRect(p.x()-NODE_RADIUS-NODE_SEL_DIST, p.y()-NODE_RADIUS-NODE_SEL_DIST,
                                    2*(NODE_RADIUS+NODE_SEL_DIST)+1, 2*(NODE_RADIUS+NODE_SEL_DIST)+1,
                                    1, 1);
        }
        if (prev != NULL) {
            davinci.setPen(lineCol);
            davinci.drawLine(convertTimeToCanvas(*prev), p);
        }

        // Handles
        if (curr->leftCurveType() != Node_sV::CurveType_Linear || true) {
            QPoint h1 = convertTimeToCanvas(*curr + Node_sV(curr->leftNodeHandle().x, curr->leftNodeHandle().y));
            QPoint h2 = convertTimeToCanvas(*curr + Node_sV(curr->rightNodeHandle().x, curr->rightNodeHandle().y));
            davinci.drawEllipse(h1.x(), h1.y(), 5, 5);
            davinci.drawEllipse(h2.x(), h2.y(), 5, 5);
        }

        prev = &m_nodes->at(i);
    }

    if (m_showHelp) {
        MainWindow::displayHelp(davinci);
    }
}

void Canvas::drawModes(QPainter &davinci, int t, int r)
{
    qreal opacity = davinci.opacity();
    int w = 16;
    int d = 8;
    int dR = 0;

    dR += w;

    davinci.setOpacity(.5 + ((m_mode == ToolMode_Select) ? .5 : 0));
    davinci.drawImage(r - dR, t, QImage("res/iconSel.png").scaled(16, 16));
    dR += d+w;

    davinci.setOpacity(.5 + ((m_mode == ToolMode_Move) ? .5 : 0));
    davinci.drawImage(r - dR, t, QImage("res/iconMov.png").scaled(16, 16));

    davinci.setOpacity(opacity);
}

void Canvas::mousePressEvent(QMouseEvent *e)
{
    m_states.reset();
    m_states.prevMousePos = e->pos();
    m_states.initialMousePos = e->pos();
    m_states.initialModifiers = e->modifiers();
    m_states.initialButtons = e->buttons();
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    m_mouseWithinWidget = true;

    m_states.travel((m_states.prevMousePos - e->pos()).manhattanLength());
    m_states.prevMousePos = e->pos();

    if (e->buttons().testFlag(Qt::LeftButton)) {

        qDebug() << m_states.initialMousePos << "to" << e->pos();
        Node_sV diff = convertCanvasToTime(e->pos()) - convertCanvasToTime(m_states.initialMousePos);
        qDebug() << "Diff: " << diff;

        if (m_mode == ToolMode_Select) {
            if (!m_states.moveAborted) {
                if (m_states.countsAsMove()) {
                    if (!m_states.selectAttempted) {
                        m_states.selectAttempted = true;
                        selectAt(m_states.initialMousePos, e->modifiers().testFlag(Qt::ControlModifier));
                    }
                    if (e->modifiers().testFlag(Qt::ControlModifier)) {
                        if (qAbs(diff.x()) < qAbs(diff.y())) {
                            diff.setX(0);
                        } else {
                            diff.setY(0);
                        }
                    }
                    m_nodes->moveSelected(diff);
                }
            }
            m_states.nodesMoved = true;
        } else if (m_mode == ToolMode_Move) {
            if (!m_states.moveAborted) {
                m_nodes->shift(convertCanvasToTime(m_states.initialMousePos).x(), diff.x());
            }
            m_states.nodesMoved = true;
        }
    }

    emit signalMouseInputTimeChanged(
                  convertCanvasToTime(m_states.prevMousePos).y()
                * m_project->frameSource()->fps()
                                     );

    repaint();
}

void Canvas::mouseReleaseEvent(QMouseEvent *)
{
    if (m_states.initialButtons.testFlag(Qt::LeftButton)) {
        if (!m_states.moveAborted) {
            switch (m_mode) {
            case ToolMode_Select:
                if (m_states.countsAsMove()) {
                    m_nodes->confirmMove();
                    qDebug() << "Move confirmed.";
                } else {
                    if (m_states.initialMousePos.x() >= m_distLeft && m_states.initialMousePos.y() < this->height()-m_distBottom) {
                        // Try to select a node below the mouse. If there is none, add a point.
                        bool selected = selectAt(m_states.initialMousePos, m_states.initialModifiers.testFlag(Qt::ControlModifier));
                        if (!selected) {
                            if (m_mode == ToolMode_Select) {
                                Node_sV p = convertCanvasToTime(m_states.initialMousePos);
                                m_nodes->add(p);
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
                }
                break;
            case ToolMode_Move:
                m_nodes->confirmMove();
                qDebug() << "Move confirmed.";
                break;
            }
        }
    } else if (m_states.initialButtons.testFlag(Qt::RightButton)) {
        Node_sV time = convertCanvasToTime(m_states.prevMousePos);
        NodeContext context = m_nodes->context(time.x(), time.y(), convertCanvasToTime(QPoint(m_distLeft+5,0)).x());
        qDebug() << "Context at " << time << " is: " << toString(context);
    }
}

void Canvas::leaveEvent(QEvent *)
{
    m_mouseWithinWidget = false;
    repaint();
}

void Canvas::wheelEvent(QWheelEvent *e)
{
    // Mouse wheel movement in degrees
    int deg = e->delta()/8;

    if (e->modifiers().testFlag(Qt::ControlModifier)) {
        Node_sV n0 = convertCanvasToTime(e->pos());

        // Update the line resolution
        m_secResX += deg;
        m_secResY += deg;
        if (m_secResX < 4) { m_secResX = 4; }
        if (m_secResY < 4) { m_secResY = 4; }

        // Adjust t0 such that the mouse points to the same time as before
        Node_sV nDiff = convertCanvasToTime(e->pos()) - convertCanvasToTime(QPoint(m_distLeft, height()-1-m_distBottom));
        m_t0 = n0 - nDiff;
        if (m_t0.x() < 0) { m_t0.setX(0); }
        if (m_t0.y() < 0) { m_t0.setY(0); }
    } else if (e->modifiers().testFlag(Qt::ShiftModifier)) {
        //Vertical scrolling
        m_t0 += Node_sV(0, (convertCanvasToTime(QPoint(deg, 0)) - convertCanvasToTime(QPoint(0,0))).x());
        if (m_t0.y() < 0) { m_t0.setY(0); }
        if (m_t0.y() > m_tmax.y()) { m_t0.setY(m_tmax.y()); }
    } else {
        // Horizontal scrolling
        m_t0 -= Node_sV((convertCanvasToTime(QPoint(deg, 0)) - convertCanvasToTime(QPoint(0,0))).x(),0);
        if (m_t0.x() < 0) { m_t0.setX(0); }
    }

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);
    Q_ASSERT(m_t0.x() >= 0);
    Q_ASSERT(m_t0.y() >= 0);

    repaint();
}

/// \todo something without m_distLeft
const Node_sV Canvas::convertCanvasToTime(const QPoint &p) const
{
    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);

    int x = p.x()-m_distLeft;
    int y = height()-1 - m_distBottom - p.y();
    Node_sV out(
                m_t0.x() + float(x) / m_secResX,
                m_t0.y() + float(y) / m_secResY
            );
    return out;
}

const QPoint Canvas::convertTimeToCanvas(const Node_sV &p) const
{
    QPoint out(
                (p.x()-m_t0.x())*m_secResX + m_distLeft,
                this->height()-1 - m_distBottom - (p.y()-m_t0.y())*m_secResY
           );
    return out;
}


void Canvas::slotAbort(Canvas::Abort abort)
{
    qDebug() << "Signal: " << abort;
    switch (abort) {
    case Abort_General:
        m_states.moveAborted = true;
        m_nodes->abortMove();
        repaint();
        break;
    case Abort_Selection:
        m_nodes->unselectAll();
        repaint();
        break;
    }

}

void Canvas::slotAddTag()
{
    if (m_mouseWithinWidget) {
        TagAddDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            m_tags->push_back(Tag_sV(convertCanvasToTime(m_states.prevMousePos).y(), dialog.m_text));
            qDebug() << "Tag added. Number is now: " << m_tags->size();
            repaint();
        } else {
            qDebug() << "Tag dialog not accepted.";
        }
    } else {
        qDebug() << "Mouse outside widget.";
    }
}

void Canvas::slotDeleteNodes()
{
    qDebug() << "Will delete";
    uint nDel = m_nodes->deleteSelected();
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
    case Canvas::ToolMode_Select:
        qd << "Select tool";
        break;
    case Canvas::ToolMode_Move:
        qd << "Move tool";
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
