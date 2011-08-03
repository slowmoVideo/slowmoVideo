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
#include "shutterFunctionDialog.h"

#include "project/projectPreferences_sV.h"
#include "project/abstractFrameSource_sV.h"

#include <cmath>
#include <typeinfo>

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


//#define DEBUG_C
#ifdef DEBUG_C
#include <iostream>
#endif

QColor Canvas::selectedCol  (  0, 175, 255, 100);
QColor Canvas::hoverCol     (255, 175,   0, 200);
QColor Canvas::lineCol      (255, 255, 255);
QColor Canvas::nodeCol      (240, 240, 240);
QColor Canvas::gridCol      (255, 255, 255,  30);
QColor Canvas::fatGridCol   (255, 255, 255,  60);
QColor Canvas::minGridCol   (200, 200, 255, 150);
QColor Canvas::handleLineCol(255, 255, 255, 128);
QColor Canvas::srcTagCol    ( 30, 245,   0, 150);
QColor Canvas::outTagCol    ( 30, 245,   0, 150);
QColor Canvas::backgroundCol( 34,  34,  34);

/// \todo move with MMB
/// \todo replay curve

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
    m_shutterFunctionDialog = NULL;

    // Enable mouse tracking (not only when a mouse button is pressed)
    this->setMouseTracking(true);

    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_states.prevMousePos = QPoint(0,0);
    m_states.initialContextObject = NULL;

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);

    m_aDeleteNode = new QAction("Delete node", this);
    m_aSnapInNode = new QAction("Snap in node", this);

    m_curveTypeMapper = new QSignalMapper(this);
    m_aLinear = new QAction("Linear curve", this);
    m_aBezier = new QAction(QString::fromUtf8("Bézier curve"), this);
    m_curveTypeMapper->setMapping(m_aLinear, CurveType_Linear);
    m_curveTypeMapper->setMapping(m_aBezier, CurveType_Bezier);

    m_a1xSpeed = new QAction(QString::fromUtf8("Set speed to 1×"), this);
    m_aShutterFunction = new QAction("Set/edit shutter function", this);

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
    b &= connect(m_a1xSpeed, SIGNAL(triggered()), this, SLOT(slotSet1xSpeed()));
    b &= connect(m_aShutterFunction, SIGNAL(triggered()), this, SLOT(slotSetShutterFunction()));
    Q_ASSERT(b);
}

Canvas::~Canvas()
{
    delete ui;
    if (m_shutterFunctionDialog != NULL) {
        delete m_shutterFunctionDialog;
    }
}

void Canvas::load(Project_sV *project)
{
    if (m_shutterFunctionDialog != NULL) {
        m_shutterFunctionDialog->close();
        delete m_shutterFunctionDialog;
        m_shutterFunctionDialog = NULL;
    }

    m_project = project;
    m_t0 = m_project->preferences()->viewport_t0();
    m_secResX = m_project->preferences()->viewport_secRes().x();
    m_secResY = m_project->preferences()->viewport_secRes().y();
    qDebug() << "Canvas: Project loaded from " << project;
    m_nodes = project->nodes();
    m_tags = project->tags();
    qDebug() << "Frame source: " << project->frameSource();
    m_tmax.setY(project->frameSource()->maxTime());
    qDebug() << "tMaxY set to " << m_tmax.y();


    repaint();
}

void Canvas::toggleHelp()
{
    m_showHelp = !m_showHelp;
    repaint();
}

const QPointF Canvas::prevMouseTime() const
{
    return convertCanvasToTime(m_states.prevMousePos).toQPointF();
}

const float Canvas::prevMouseInFrame() const
{
    return convertCanvasToTime(m_states.prevMousePos).toQPointF().y() * m_project->frameSource()->fps()->fps();
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
    davinci.setRenderHint(QPainter::Antialiasing, false);
    davinci.fillRect(0, 0, width(), height(), backgroundCol);

    QList<NodeList_sV::PointerWithDistance> nearObjects = m_nodes->objectsNear(
                convertCanvasToTime(m_states.prevMousePos).toQPointF(),
                delta(SELECT_RADIUS));
    if (m_states.prevModifiers.testFlag(Qt::ShiftModifier)) {
        while (nearObjects.size() > 0 && nearObjects.at(0).type == NodeList_sV::PointerWithDistance::Node) {
            nearObjects.removeFirst();
        }
    }

    bool drawLine;
    // x grid
    for (int tx = ceil(m_t0.x()); true; tx++) {
        QPoint pos = convertTimeToCanvas(Node_sV(tx, m_t0.y()));
        if (insideCanvas(pos)) {
            drawLine = m_secResX >= 4;
            if (tx%60 == 0) {
                davinci.setPen(minGridCol);
                drawLine = true;
            } else if (tx%10 == 0) {
                davinci.setPen(fatGridCol);
                drawLine = m_secResX >= .7;
            } else {
                davinci.setPen(gridCol);
            }

            if (drawLine) {
                davinci.drawLine(pos.x(), pos.y(), pos.x(), m_distTop);
            }
        } else {
            break;
        }
    }
    // y grid
    for (int ty = ceil(m_t0.y()); true; ty++) {
        QPoint pos = convertTimeToCanvas(Node_sV(m_t0.x(), ty));
        if (insideCanvas(pos)) {
            drawLine = m_secResY >= 4;
            if (ty%60 == 0) {
                davinci.setPen(minGridCol);
                drawLine = true;
            } else if (ty%10 == 0) {
                davinci.setPen(fatGridCol);
                drawLine = m_secResX >= .7;
            } else {
                davinci.setPen(gridCol);
            }

            if (drawLine) {
                davinci.drawLine(pos.x(), pos.y(), width()-1 - m_distRight, pos.y());
            }
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
        QString timeText;
        Node_sV time = convertCanvasToTime(m_states.prevMousePos);
        int posX;

        davinci.drawLine(m_states.prevMousePos.x(), m_distTop, m_states.prevMousePos.x(), height()-1 - m_distBottom);
        if (time.x() < 60) {
            timeText = QString("%1 s").arg(time.x());
        } else {
            timeText = QString("%1 min %2 s").arg(int(time.x()/60)).arg(time.x()-60*int(time.x()/60), 0, 'f', 3);
        }
        // Ensure that the text does not go over the right border
        posX = m_states.prevMousePos.x() - 20;
        if (posX > width()-m_distLeft-40) {
            posX = width()-m_distLeft-40;
        }
        davinci.drawText(posX, height()-1 - 20, timeText);
        davinci.drawLine(m_distLeft, m_states.prevMousePos.y(), m_states.prevMousePos.x(), m_states.prevMousePos.y());
        if (time.y() < 60) {
            timeText = QString("f %1\n%2 s")
                    .arg(time.y()*m_project->frameSource()->fps()->fps(), 2, 'f', 2)
                    .arg(time.y());
        } else {
            timeText = QString("f %1\n%2 min\n+%3 s")
                    .arg(time.y()*m_project->frameSource()->fps()->fps(), 2, 'f', 2)
                    .arg(int(time.y()/60))
                    .arg(time.y()-60*int(time.y()/60), 0, 'f', 2);
        }
        davinci.drawText(8, m_states.prevMousePos.y()-6, m_distLeft-2*8, 50, Qt::AlignRight, timeText);
    }
    int bottom = height()-1 - m_distBottom;
    davinci.drawLine(m_distLeft, bottom, width()-1 - m_distRight, bottom);
    davinci.drawLine(m_distLeft, bottom, m_distLeft, m_distTop);

    // Tags
    davinci.setRenderHint(QPainter::Antialiasing, false);
    for (int i = 0; i < m_tags->size(); i++) {
        Tag_sV tag = m_tags->at(i);
        if (tag.axis() == TagAxis_Source) {
            QPoint p = convertTimeToCanvas(Node_sV(m_t0.x(), tag.time()));
            if (insideCanvas(p)) {
                davinci.setPen(srcTagCol);
                davinci.drawLine(m_distLeft, p.y(), width()-m_distRight, p.y());
                davinci.drawText(m_distLeft+10, p.y()-1, tag.description());
            }
        } else {
            QPoint p = convertTimeToCanvas(Node_sV(tag.time(), m_t0.y()));
            if (insideCanvas(p)) {
                davinci.setPen(outTagCol);
                davinci.drawLine(p.x(), height()-1 - m_distBottom, p.x(), m_distTop);
                davinci.drawText(p.x()+2, m_distTop, tag.description());
            }
        }
    }

    // Nodes
    davinci.setPen(lineCol);
    davinci.setRenderHint(QPainter::Antialiasing, true);
    const Node_sV *prev = NULL;
    const Node_sV *curr = NULL;
    for (int i = 0; i < m_nodes->size(); i++) {
        curr = &m_nodes->at(i);

        QPoint p = convertTimeToCanvas(*curr);

        if (curr->selected()) {
            davinci.setPen(QPen(QBrush(selectedCol), 2.0));
            davinci.fillRect(p.x()-NODE_RADIUS, p.y()-NODE_RADIUS, 2*NODE_RADIUS+1, 2*NODE_RADIUS+1, selectedCol);
        }
        davinci.setPen(nodeCol);
        if (nearObjects.size() > 0 && curr == nearObjects.at(0).ptr) {
            davinci.setPen(hoverCol);
        }
        davinci.drawRect(p.x()-NODE_RADIUS, p.y()-NODE_RADIUS, 2*NODE_RADIUS+1, 2*NODE_RADIUS+1);
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

        // Handles
        if (i > 0 && curr->leftCurveType() != CurveType_Linear && prev->rightCurveType() != CurveType_Linear) {
            davinci.setPen(handleLineCol);
            if (nearObjects.size() > 0 && &curr->leftNodeHandle() == nearObjects.at(0).ptr) {
                davinci.setPen(hoverCol);
            }
            QPoint h = convertTimeToCanvas(curr->toQPointF() + curr->leftNodeHandle());
            davinci.drawLine(convertTimeToCanvas(*curr), h);
            davinci.drawEllipse(QPoint(h.x(), h.y()), HANDLE_RADIUS, HANDLE_RADIUS);

            davinci.setPen(handleLineCol);
            if (nearObjects.size() > 0 && &prev->rightNodeHandle() == nearObjects.at(0).ptr) {
                davinci.setPen(hoverCol);
            }
            h = convertTimeToCanvas(prev->toQPointF() + prev->rightNodeHandle());
            davinci.drawLine(convertTimeToCanvas(*prev), h);
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
    m_states.reset();

    m_states.prevMousePos = e->pos();
    m_states.initialMousePos = e->pos();

    m_states.prevModifiers = e->modifiers();
    m_states.initialModifiers = e->modifiers();
    m_states.initialButtons = e->buttons();

    m_states.initialContextObject = objectAt(e->pos(), e->modifiers());

    if (m_states.initialContextObject != NULL) {
        qDebug() << "Mouse pressed. Context: " << typeid(*m_states.initialContextObject).name();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *e)
{
    m_mouseWithinWidget = true;

    m_states.travel((m_states.prevMousePos - e->pos()).manhattanLength());
    m_states.prevMousePos = e->pos();
    m_states.prevModifiers = e->modifiers();

    if (e->buttons().testFlag(Qt::LeftButton)) {

        Node_sV diff = convertCanvasToTime(e->pos()) - convertCanvasToTime(m_states.initialMousePos);
#ifdef DEBUG_C
        qDebug() << m_states.initialMousePos << "to" << e->pos() << "; Diff: " << diff;
#endif

        if (m_mode == ToolMode_Select) {
            if (dynamic_cast<const NodeHandle_sV*>(m_states.initialContextObject) != NULL) {
                const NodeHandle_sV *handle = dynamic_cast<const NodeHandle_sV*>(m_states.initialContextObject);
                int index = m_nodes->indexOf(handle->parentNode());
                if (index < 0) {
                    qDebug () << "FAIL!";
                }
                qDebug() << "Moving handle" << handle << " of node " << handle->parentNode()
                         << QString(" (%1)").arg(index);
                qDebug() << "Parent node x: " << handle->parentNode()->x();
                qDebug() << "Handle x: " << handle->x();

                if (index >= 0) {
                    m_nodes->moveHandle(
                                handle,
                                convertCanvasToTime(e->pos())-m_nodes->at(index)
                                );
                } else {
                    for (int i = 0; i < m_nodes->size(); i++) {
                        qDebug() << "Node " << i << " is at " << &m_nodes->at(i);
                    }
                }
            } else if (dynamic_cast<const Node_sV*>(m_states.initialContextObject) != NULL) {
                const Node_sV *node = (const Node_sV*) m_states.initialContextObject;

                if (!m_states.nodesMoved) {
                    qDebug() << "Moving node " << node;
                }
                if (!m_states.moveAborted) {
                    if (m_states.countsAsMove()) {
                        if (!node->selected()) {
                            if (!m_states.selectAttempted) {
                                m_states.selectAttempted = true;
                                m_nodes->select(node, !e->modifiers().testFlag(Qt::ControlModifier));
                            }
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
            } else {
                if (m_states.initialContextObject != NULL) {
                    qDebug() << "Trying to move " << typeid(*m_states.initialContextObject).name() << ": Not supported yet!";
                }
            }

        } else if (m_mode == ToolMode_Move) {
            if (!m_states.moveAborted) {
                m_nodes->shift(convertCanvasToTime(m_states.initialMousePos).x(), diff.x());
            }
            m_states.nodesMoved = true;
        }
    }

    // Emit the source time at the mouse position
    emit signalMouseInputTimeChanged(
                  convertCanvasToTime(m_states.prevMousePos).y()
                * m_project->frameSource()->fps()->fps()
                                     );


    // Emit the source time at the intersection of the out time and the curve
    qreal timeOut = convertCanvasToTime(m_states.prevMousePos).x();
    if (m_nodes->size() > 1 && m_nodes->startTime() <= timeOut && timeOut <= m_nodes->endTime()) {

#ifdef DEBUG_C
        std::cout.precision(32);
        std::cout << "start: " << m_nodes->startTime() << ", out: " << timeOut << ", end: " << m_nodes->endTime() << std::endl;
#endif

        if (m_nodes->find(timeOut) >= 0) {
            emit signalMouseCurveSrcTimeChanged(
                        m_nodes->sourceTime(timeOut)
                      * m_project->frameSource()->fps()->fps());
        }
    }

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
                    emit nodesChanged();
                } else {
                    if (m_states.initialMousePos.x() >= m_distLeft && m_states.initialMousePos.y() < this->height()-m_distBottom
                            && !m_states.selectAttempted) {

                        // Try to select a node below the mouse. If there is none, add a point.
                        if (m_states.initialContextObject == NULL || dynamic_cast<const Node_sV*>(m_states.initialContextObject) == NULL) {
                            if (m_mode == ToolMode_Select) {
                                Node_sV p = convertCanvasToTime(m_states.initialMousePos);
                                m_nodes->add(p);
                                emit nodesChanged();
                            } else {
                                qDebug() << "Not adding node. Mode is " << m_mode;
                            }

                        } else if (dynamic_cast<const Node_sV*>(m_states.initialContextObject) != NULL) {
                            m_nodes->select((const Node_sV*) m_states.initialContextObject, !m_states.initialModifiers.testFlag(Qt::ControlModifier));
                        }
                        repaint();

                    } else {
                        qDebug() << "Not inside bounds.";
                    }
                }
                break;
            case ToolMode_Move:
                m_nodes->confirmMove();
                qDebug() << "Move confirmed.";
                emit nodesChanged();
                break;
            }
        }
    } else if (m_states.initialButtons.testFlag(Qt::RightButton) || m_states.initialButtons.testFlag(Qt::MiddleButton)) {
        QList<NodeList_sV::PointerWithDistance> nearObjects = m_project->objectsNear(convertCanvasToTime(m_states.initialMousePos).toQPointF(),  delta(10));
        qDebug() << "Nearby objects:";
        for (int i = 0; i < nearObjects.size(); i++) {
            qDebug() << typeid(*(nearObjects.at(i).ptr)).name() << " at distance " << nearObjects.at(i).dist;
        }
    }
}

void Canvas::contextMenuEvent(QContextMenuEvent *e)
{
    qDebug() << "Context menu requested";
    QMenu menu;

    const CanvasObject_sV *obj = objectAt(e->pos(), m_states.prevModifiers);

    if (dynamic_cast<const Node_sV*>(obj)) {
        int nodeIndex = m_nodes->indexOf((const Node_sV*)obj);

        menu.addAction(QString("Node %1").arg(nodeIndex))->setEnabled(false);
        menu.addAction(m_aDeleteNode);
        menu.addAction(m_aSnapInNode);
        menu.addSeparator()->setText("Handle actions");
        menu.addAction(m_aResetLeftHandle);
        menu.addAction(m_aResetRightHandle);

    } else if (dynamic_cast<const Segment_sV*>(obj) != NULL) {
        const Segment_sV* segment = (const Segment_sV*) obj;
        int leftNode = segment->leftNodeIndex();

        menu.addAction(QString("Segment between node %1 and %2").arg(leftNode).arg(leftNode+1))->setEnabled(false);
        menu.addAction(m_aLinear);
        menu.addAction(m_aBezier);
        menu.addAction(m_a1xSpeed);
        menu.addAction(m_aShutterFunction);

    } else {
        if (obj != NULL) {
            qDebug() << "No context menu available for object of type " << typeid(*obj).name();
        }
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
        if (deg > 0) {
            m_secResX *= ZOOM_FACTOR;
        } else {
            m_secResX /= ZOOM_FACTOR;
        }
        if (m_secResX < .05) { m_secResX = .05; }
        // Y resolution is the same as X resolution (at least at the moment)
        m_secResY = m_secResX;

        // Adjust t0 such that the mouse points to the same time as before
        Node_sV nDiff = convertCanvasToTime(e->pos()) - convertCanvasToTime(QPoint(m_distLeft, height()-1-m_distBottom));
        m_t0 = n0 - nDiff;
        if (m_t0.x() < 0) { m_t0.setX(0); }
        if (m_t0.y() < 0) { m_t0.setY(0); }
    } else if (e->modifiers().testFlag(Qt::ShiftModifier)) {
        // Horizontal scrolling
        m_t0 -= Node_sV(SCROLL_FACTOR*convertDistanceToTime(QPoint(deg, 0)).x(),0);
        if (m_t0.x() < 0) { m_t0.setX(0); }
    } else {
        //Vertical scrolling
        m_t0 += Node_sV(0, SCROLL_FACTOR*convertDistanceToTime(QPoint(deg, 0)).x());
        if (m_t0.y() < 0) { m_t0.setY(0); }
        if (m_t0.y() > m_tmax.y()) { m_t0.setY(m_tmax.y()); }
    }


    m_project->preferences()->viewport_t0() = m_t0.toQPointF();
    m_project->preferences()->viewport_secRes().rx() = m_secResX;
    m_project->preferences()->viewport_secRes().ry() = m_secResY;

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);
    Q_ASSERT(m_t0.x() >= 0);
    Q_ASSERT(m_t0.y() >= 0);

    repaint();
}



const CanvasObject_sV* Canvas::objectAt(QPoint pos, Qt::KeyboardModifiers modifiers) const
{
    QList<NodeList_sV::PointerWithDistance> nearObjects =
            m_project->objectsNear(convertCanvasToTime(pos).toQPointF(), convertDistanceToTime(QPoint(SELECT_RADIUS,0)).x());

    if (modifiers.testFlag(Qt::ShiftModifier)) {
        // Ignore nodes with Shift pressed
        while (nearObjects.size() > 0 && dynamic_cast<const Node_sV*>(nearObjects.at(0).ptr) != NULL) {
            nearObjects.removeFirst();
        }
    }

    if (nearObjects.size() > 0) {
        return nearObjects.at(0).ptr;
    } else {
        return NULL;
    }
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
        TagAddDialog dialog(m_project->preferences()->lastSelectedTagAxis(), this);

        if (dialog.exec() == QDialog::Accepted) {
            Tag_sV tag = dialog.buildTag(convertCanvasToTime(m_states.prevMousePos).toQPointF());
            m_project->preferences()->lastSelectedTagAxis() = tag.axis();

            m_tags->push_back(tag);
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
        emit nodesChanged();
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
        emit nodesChanged();
    }
}
void Canvas::slotSnapInNode()
{
    /// \todo implement
    qDebug() << "Snapping in at " << m_states.prevMousePos;
}
void Canvas::slotChangeCurveType(int curveType)
{
    qDebug() << "Changing curve type to " << toString((CurveType)curveType) << " at " << convertCanvasToTime(m_states.prevMousePos).x();
    m_nodes->setCurveType(convertCanvasToTime(m_states.prevMousePos).x(), (CurveType) curveType);
    emit nodesChanged();
}
void Canvas::slotResetHandle(const QString &position)
{
    if (dynamic_cast<const Node_sV*>(m_states.initialContextObject) != NULL) {
        Node_sV *node = const_cast<Node_sV*>(dynamic_cast<const Node_sV*>(m_states.initialContextObject));
        if (position == "left") {
            node->setLeftNodeHandle(0, 0);
        } else {
            node->setRightNodeHandle(0, 0);
        }
        emit nodesChanged();
    } else {
        qDebug() << "Object at mouse position is " << m_states.initialContextObject << ", cannot reset the handle.";
    }
}
void Canvas::slotSet1xSpeed()
{
    qDebug() << "Setting curve to 1x speed.";
    m_nodes->set1xSpeed(convertCanvasToTime(m_states.prevMousePos).x());
    emit nodesChanged();
}
void Canvas::slotSetShutterFunction()
{
    int left = m_nodes->find(convertDistanceToTime(QPoint(m_states.prevMousePos.x(), 0)).x());
    if (left == m_nodes->size()-1) {
        left = m_nodes->size()-2;
    }

    if (m_shutterFunctionDialog == NULL) {
        m_shutterFunctionDialog = new ShutterFunctionDialog(m_project, this);
        bool b = true;
        b &= connect(this, SIGNAL(nodesChanged()), m_shutterFunctionDialog, SLOT(slotNodesUpdated()));
        Q_ASSERT(b);
    }

    m_shutterFunctionDialog->setSegment(left);
    if (!m_shutterFunctionDialog->isVisible()) {
        m_shutterFunctionDialog->show();
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
