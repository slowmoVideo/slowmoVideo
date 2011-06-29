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

#include <QtCore/QDebug>

#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QSignalMapper>

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QMenu>

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

    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_states.prevMousePos = QPoint(0,0);

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);

    m_aDeleteNode = new QAction("Delete node", this);
    m_aSnapInNode = new QAction("Snap in node", this);

    m_curveTypeMapper = new QSignalMapper(this);
    m_aLinear = new QAction("Linear curve", this);
    m_aBezier = new QAction(QString::fromUtf8("Bézier curve"), this);
    m_curveTypeMapper->setMapping(m_aLinear, CurveType_Linear);
    m_curveTypeMapper->setMapping(m_aBezier, CurveType_Bezier);

    m_handleMapper = new QSignalMapper(this);
    m_aResetLeftHandle = new QAction("Reset left handle", this);
    m_aResetRightHandle = new QAction("Reset right handle", this);
    m_handleMapper->setMapping(m_aResetLeftHandle, "left");
    m_handleMapper->setMapping(m_aResetRightHandle, "right");


    bool b = true;
    b &= connect(m_aDeleteNode, SIGNAL(triggered()), this, SLOT(slotDeleteNode()));
    b &= connect(m_aSnapInNode, SIGNAL(triggered()), this, SLOT(slotSnapInNode()));
    b &= connect(m_aLinear, SIGNAL(triggered()), m_curveTypeMapper, SLOT(map()));
    b &= connect(m_aBezier, SIGNAL(triggered()), m_curveTypeMapper, SLOT(map()));
    b &= connect(m_curveTypeMapper, SIGNAL(mapped(int)), this, SLOT(slotChangeCurveType(int)));
    b &= connect(m_aResetLeftHandle, SIGNAL(triggered()), m_handleMapper, SLOT(map()));
    b &= connect(m_aResetRightHandle, SIGNAL(triggered()), m_handleMapper, SLOT(map()));
    b &= connect(m_handleMapper, SIGNAL(mapped(QString)), this, SLOT(slotResetHandle(QString)));
    Q_ASSERT(b);
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
                abs(p.x() - pos.x()) <= NODE_RADIUS+SELECT_RADIUS+4 &&
                abs(p.y() - pos.y()) <= NODE_RADIUS+SELECT_RADIUS+4
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
            davinci.drawRoundedRect(p.x()-NODE_SELECTED_RADIUS, p.y()-NODE_SELECTED_RADIUS,
                                    2*NODE_SELECTED_RADIUS+1, 2*NODE_SELECTED_RADIUS+1,
                                    1, 1);
        }
        if (prev != NULL) {
            davinci.setPen(lineCol);
            if (prev->rightCurveType() == CurveType_Bezier && curr->leftCurveType() == CurveType_Bezier) {
                QPainterPath path;
                path.moveTo(convertTimeToCanvas(*prev));
                path.cubicTo(
                            convertTimeToCanvas(prev->toQPointF() + prev->rightNodeHandle()),
                            convertTimeToCanvas(curr->toQPointF() + curr->leftNodeHandle()),
                            convertTimeToCanvas(*curr));
                davinci.drawPath(path);
            } else {
                davinci.drawLine(convertTimeToCanvas(*prev), p);
            }
        }

//        // Handles
//        if (i > 0 && curr->leftCurveType() != CurveType_Linear && prev->rightCurveType() != CurveType_Linear) {
//            // TODO improve
//            QPoint h = convertTimeToCanvas(curr->toQPointF() + curr->leftNodeHandle());
//            davinci.drawLine(convertTimeToCanvas(*curr), h);
//            davinci.drawEllipse(QPoint(h.x(), h.y()), HANDLE_RADIUS, HANDLE_RADIUS);
//            h = convertTimeToCanvas(prev->toQPointF() + prev->rightNodeHandle());
//            davinci.drawLine(convertTimeToCanvas(*prev), h);
//            davinci.drawEllipse(QPoint(h.x(), h.y()), HANDLE_RADIUS, HANDLE_RADIUS);
//        }
        // Handles
        if (curr->leftCurveType() != CurveType_Linear && i > 0) {
            QPoint h = convertTimeToCanvas(curr->toQPointF() + curr->leftNodeHandle());
            davinci.drawEllipse(QPoint(h.x(), h.y()), HANDLE_RADIUS, HANDLE_RADIUS);
        }
        if (curr->rightCurveType() != CurveType_Linear && i < m_nodes->size()-1) {
            QPoint h = convertTimeToCanvas(curr->toQPointF() + curr->rightNodeHandle());
            davinci.drawEllipse(QPoint(h.x(), h.y()), HANDLE_RADIUS, HANDLE_RADIUS);
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
    QPointF time = convertCanvasToTime(e->pos()).toQPointF();
    m_states.reset();
    m_states.prevMousePos = e->pos();
    m_states.initialMousePos = e->pos();
    m_states.initialModifiers = e->modifiers();
    m_states.initialButtons = e->buttons();
    m_states.context = m_nodes->context(time, delta(SELECT_RADIUS));
    if (m_states.context == NodeContext_Handle) {
        m_states.nodeOfHandle = m_nodes->findByHandle(time.x(), time.y(), delta(SELECT_RADIUS));
        m_states.leftHandle = e->pos().x() < convertTimeToCanvas(m_nodes->at(m_states.nodeOfHandle)).x();
    }
    qDebug() << "Mouse pressed. Context: " << toString(m_states.context);
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
            if (m_states.context == NodeContext_Handle) {
                qDebug() << "Moving handle" << m_states.nodeOfHandle << "; left? " << m_states.leftHandle;
                if (m_states.nodeOfHandle >= 0) {
                    m_nodes->moveHandle(
                                m_states.nodeOfHandle,
                                m_states.leftHandle,
                                convertCanvasToTime(e->pos())-m_nodes->at(m_states.nodeOfHandle)
                                );
                }
            } else {
                qDebug() << "Moving node.";
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
            }

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

void Canvas::contextMenuEvent(QContextMenuEvent *e)
{
    qDebug() << "Context menu requested";
    QMenu menu;
    NodeContext context = m_nodes->context(
                convertCanvasToTime(e->pos()).toQPointF(),
                convertCanvasToTime(QPoint(m_distLeft+5,0)).x()
                );
    switch(context) {
    case NodeContext_Node: {
        int nodeIndex = m_nodes->find(convertCanvasToTime(m_states.prevMousePos).toQPointF(), delta(SELECT_RADIUS));
        menu.addAction(QString("Node %1").arg(nodeIndex))->setEnabled(false);
        menu.addAction(m_aDeleteNode);
        menu.addAction(m_aSnapInNode);
        break; }
    case NodeContext_Segment: {
        int leftNode, rightNode;
        m_nodes->findBySegment(convertCanvasToTime(m_states.prevMousePos).x(), leftNode, rightNode);

        menu.addAction(QString("Segment between node %1 and %2").arg(leftNode).arg(rightNode))->setEnabled(false);
        menu.addAction(m_aLinear);
        menu.addAction(m_aBezier);
        menu.addSeparator()->setText("Handle actions");
        menu.addAction(m_aResetLeftHandle);
        menu.addAction(m_aResetRightHandle);
        break; }
    default:
        qDebug() << "No context menu available for context " << toString(context);
        return;
    }
    menu.exec(e->globalPos());
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



////////// Conversion Time <--> Screen pixels

Node_sV Canvas::convertCanvasToTime(const QPoint &p) const
{
    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);

    QPointF tDelta = convertDistanceToTime(QPoint(
                                                       p.x()-m_distLeft,
                                                       height()-1 - m_distBottom - p.y()
                                                       ));
    QPointF tFinal = tDelta + m_t0.toQPointF();
    return Node_sV(tFinal.x(), tFinal.y());
}
QPoint Canvas::convertTimeToCanvas(const Node_sV &p) const
{
    return convertTimeToCanvas(p.toQPointF());
}
QPoint Canvas::convertTimeToCanvas(const QPointF &p) const
{
    QPoint tDelta = convertTimeToDistance(QPointF(
                                              p.x()-m_t0.x(),
                                              p.y()-m_t0.y()
                                              ));
    QPoint out(
                tDelta.x() + m_distLeft,
                height()-1 - m_distBottom - tDelta.y()
                );
    return out;
}
QPointF Canvas::convertDistanceToTime(const QPoint &p) const
{
    QPointF out(
                float(p.x()) / m_secResX,
                float(p.y()) / m_secResY
            );
    return out;
}
QPoint Canvas::convertTimeToDistance(const QPointF &time) const
{
    QPoint out(
                time.x()*m_secResX,
                time.y()*m_secResY
           );
    return out;
}
float Canvas::delta(int px) const
{
    return convertDistanceToTime(QPoint(px, 0)).x();
}



////////// Slots

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

void Canvas::slotDeleteNode()
{
    qDebug() << "Deleting node at " << m_states.prevMousePos;
    int index = m_nodes->find(convertCanvasToTime(m_states.prevMousePos).toQPointF(), delta(SELECT_RADIUS));
    if (index >= 0) {
        m_nodes->deleteNode(index);
    }
}
void Canvas::slotSnapInNode()
{
    qDebug() << "Snapping in at " << m_states.prevMousePos;
}
void Canvas::slotChangeCurveType(int curveType)
{
    qDebug() << "Changing curve type to " << toString((CurveType)curveType) << " at " << convertCanvasToTime(m_states.prevMousePos).x();
    m_nodes->setCurveType(convertCanvasToTime(m_states.prevMousePos).x(), (CurveType) curveType);
}
void Canvas::slotResetHandle(const QString &position)
{
    int leftNode, rightNode;
    m_nodes->findBySegment(convertCanvasToTime(m_states.prevMousePos).x(), leftNode, rightNode);
    if (position == "left") {
        if (leftNode >= 0) {
            m_nodes->moveHandle(leftNode, false, Node_sV());
        }
    } else if (position == "right") {
        if (rightNode >= 0) {
            m_nodes->moveHandle(rightNode, true, Node_sV());
        }
    } else {
        qDebug() << "Unknown handle position: " << position;
        Q_ASSERT(false);
    }
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
