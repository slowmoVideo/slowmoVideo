/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "canvas.h"
#include "canvasTools.h"
#include "ui_canvas.h"

#include "mainwindow.h"
#include "tagAddDialog.h"
#include "shutterFunctionDialog.h"
#include "project/shutterFunction_sV.h"
#include "project/shutterFunctionList_sV.h"
#include "lib/bezierTools_sV.h"

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
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>


//#define VALIDATE_BEZIER
#ifdef VALIDATE_BEZIER
#include "lib/bezierTools_sV.h"
#endif

//#define DEBUG_C
#ifdef DEBUG_C
#include <iostream>
#endif

QColor Canvas::selectedCol  (  0, 175, 255, 100);
QColor Canvas::hoverCol     (255, 175,   0, 200);
QColor Canvas::lineCol      (255, 255, 255);
QColor Canvas::selectedLineCol(255, 175,   0, 200);
QColor Canvas::nodeCol      (240, 240, 240);
QColor Canvas::gridCol      (255, 255, 255,  30);
QColor Canvas::fatGridCol   (255, 255, 255,  60);
QColor Canvas::minGridCol   (200, 200, 255, 150);
QColor Canvas::handleLineCol(255, 255, 255, 128);
QColor Canvas::srcTagCol    ( 30, 245,   0, 150);
QColor Canvas::outTagCol    ( 30, 245,   0, 150);
QColor Canvas::backgroundCol( 34,  34,  34);
QColor Canvas::shutterRegionCol(175, 25, 75, 100);
QColor Canvas::shutterRegionBoundCol(240, 0, 60, 150);

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
    setFocusPolicy(Qt::StrongFocus);

    m_states.prevMousePos = QPoint(0,0);
    m_states.contextmenuMouseTime = QPointF(0,0);
    m_states.initialContextObject = NULL;

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);

    m_aDeleteNode = new QAction(tr("&Delete node"), this);
    m_aSnapInNode = new QAction(tr("&Snap in node"), this);
    m_aDeleteTag = new QAction(tr("&Delete tag"), this);
    m_aRenameTag = new QAction(tr("&Rename tag"), this);
    m_aSetTagTime = new QAction(tr("Set tag &time"), this);
    m_hackMapper = new QSignalMapper(this);
    m_hackMapper->setMapping(m_aRenameTag, &m_toRenameTag); m_toRenameTag.reason = TransferObject::ACTION_RENAME;
    m_hackMapper->setMapping(m_aDeleteTag, &m_toDeleteTag); m_toDeleteTag.reason = TransferObject::ACTION_DELETE;
    m_hackMapper->setMapping(m_aDeleteNode, &m_toDeleteNode); m_toDeleteNode.reason = TransferObject::ACTION_DELETE;
    m_hackMapper->setMapping(m_aSnapInNode, &m_toSnapInNode); m_toSnapInNode.reason = TransferObject::ACTION_SNAPIN;
    m_hackMapper->setMapping(m_aSetTagTime, &m_toSetTagTime); m_toSetTagTime.reason = TransferObject::ACTION_SETTIME;

    m_curveTypeMapper = new QSignalMapper(this);
    m_aLinear = new QAction(tr("&Linear curve"), this);
    m_aBezier = new QAction(trUtf8("&Bézier curve"), this);
    m_curveTypeMapper->setMapping(m_aLinear, CurveType_Linear);
    m_curveTypeMapper->setMapping(m_aBezier, CurveType_Bezier);

    m_aCustomSpeed = new QAction(tr("Set &custom speed"), this);
    m_aShutterFunction = new QAction(tr("Set/edit shutter &function"), this);

    m_speedsMapper = new QSignalMapper(this);
    double arr[] = {1, .5, 0, -.5, -1};
#define N_SPEEDS 5
    for (int i = 0; i < N_SPEEDS; i++) {
        m_aSpeeds.push_back(new QAction(trUtf8("Set speed to %1×").arg(arr[i], 0, 'f', 1), this));
        m_speedsMapper->setMapping(m_aSpeeds.back(), QString("%1").arg(arr[i],0,'f',1));
    }

    m_handleMapper = new QSignalMapper(this);
    m_aResetLeftHandle = new QAction(tr("Reset left handle"), this);
    m_aResetRightHandle = new QAction(tr("Reset right handle"), this);
    m_handleMapper->setMapping(m_aResetLeftHandle, "left");
    m_handleMapper->setMapping(m_aResetRightHandle, "right");


    connect(m_aSnapInNode, SIGNAL(triggered()), m_hackMapper, SLOT(map()));
    connect(m_aDeleteNode, SIGNAL(triggered()), m_hackMapper, SLOT(map()));
    connect(m_aDeleteTag, SIGNAL(triggered()), m_hackMapper, SLOT(map()));
    connect(m_aRenameTag, SIGNAL(triggered()), m_hackMapper, SLOT(map()));
    connect(m_aSetTagTime, SIGNAL(triggered()), m_hackMapper, SLOT(map()));
    connect(m_hackMapper, SIGNAL(mapped(QObject*)), this, SLOT(slotRunAction(QObject*)));
    connect(m_aLinear, SIGNAL(triggered()), m_curveTypeMapper, SLOT(map()));
    connect(m_aBezier, SIGNAL(triggered()), m_curveTypeMapper, SLOT(map()));
    connect(m_curveTypeMapper, SIGNAL(mapped(int)), this, SLOT(slotChangeCurveType(int)));
    connect(m_aResetLeftHandle, SIGNAL(triggered()), m_handleMapper, SLOT(map()));
    connect(m_aResetRightHandle, SIGNAL(triggered()), m_handleMapper, SLOT(map()));
    connect(m_handleMapper, SIGNAL(mapped(QString)), this, SLOT(slotResetHandle(QString)));
    connect(m_aCustomSpeed, SIGNAL(triggered()), this, SLOT(slotSetSpeed()));
    connect(m_speedsMapper, SIGNAL(mapped(QString)), this, SLOT(slotSetSpeed(QString)));
    connect(m_aShutterFunction, SIGNAL(triggered()), this, SLOT(slotSetShutterFunction()));
    for (std::vector<QAction*>::iterator it = m_aSpeeds.begin(); it != m_aSpeeds.end(); ++it) {
        connect(*it, SIGNAL(triggered()), m_speedsMapper, SLOT(map()));
    }
}

Canvas::~Canvas()
{
    delete ui;
    if (m_shutterFunctionDialog != NULL) {
        delete m_shutterFunctionDialog;
    }

    while (!m_aSpeeds.empty()) {
        delete m_aSpeeds.back();
        m_aSpeeds.pop_back();
    }
    delete m_speedsMapper;
    delete m_hackMapper;

    delete m_aSnapInNode;
    delete m_aDeleteNode;
    delete m_aDeleteTag;
    delete m_aRenameTag;
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
    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);
    qDebug() << "Canvas: Project loaded from " << project;
    m_nodes = project->nodes();
    m_tags = project->tags();
    qDebug() << "Frame source: " << project->frameSource();
    m_tmax.setY(project->frameSource()->maxTime());
    qDebug() << "tMaxY set to " << m_tmax.y();


    repaint();
}

void Canvas::showHelp(bool show)
{
    m_showHelp = show;
    repaint();

    m_settings.setValue("ui/displayHelp", show);
    m_settings.sync();
}

void Canvas::toggleHelp()
{
    showHelp(!m_showHelp);
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

QRect Canvas::leftDrawingRect(int y, const int height, const int min, const int max) const
{
    if (y < min) { y = min; }
    if (max > 0 && y > max-height) { y = max-height; }
    return QRect(8, y-6, m_distLeft-2*8, 50);
}
QRect Canvas::bottomDrawingRect(int x, const int width, const int min, const int max, bool rightJustified) const
{
    if (rightJustified) {
        if (max > 0 && x > max) { x = max; }
        if (min > 0 && x< min+width) { x = min+width; }
        return QRect(x-width, height()-1 - (m_distBottom-8), width, m_distBottom-2*8);
    } else {
        if (max > 0 && x > max-width) { x = max-width; }
        if (min > 0 && x < min) { x = min; }
        return QRect(x, height()-1 - (m_distBottom-8), width, m_distBottom-2*8);
    }
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
            drawLine = m_secResX >= 7.5;
            if (tx%60 == 0) {
                davinci.setPen(minGridCol);
                drawLine = true;
            } else if (tx%10 == 0) {
                davinci.setPen(fatGridCol);
                drawLine = m_secResX >= .75;
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
            drawLine = m_secResY >= 7.5;
            if (ty%60 == 0) {
                davinci.setPen(minGridCol);
                drawLine = true;
            } else if (ty%10 == 0) {
                davinci.setPen(fatGridCol);
                drawLine = m_secResX >= .75;
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
        QString timeText, speedText;
        Node_sV time = convertCanvasToTime(m_states.prevMousePos);

        const int mX = m_states.prevMousePos.x();
        const int mY = m_states.prevMousePos.y();

        davinci.drawLine(mX, m_distTop, mX, height()-1 - m_distBottom);
        timeText = CanvasTools::outputTimeLabel(this, time);
        speedText = CanvasTools::outputSpeedLabel(time, m_project);
        // Ensure that the text does not go over the right border

        davinci.drawText(bottomDrawingRect(mX-20, 160, m_distLeft, -180+width()-m_distRight-50), Qt::AlignRight, timeText);
        davinci.drawText(bottomDrawingRect(mX+20, 160, m_distLeft+180,  width()-m_distRight-50, false), Qt::AlignLeft, speedText);
        davinci.drawLine(m_distLeft, mY, mX, mY);
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
        davinci.drawText(leftDrawingRect(mY, 48, m_distTop+24, height()-m_distBottom), Qt::AlignRight, timeText);
    }
    {
        Node_sV node; QString timeText ;

        // yMax
        node = convertCanvasToTime(QPoint(m_distLeft, m_distTop));
        timeText = QString("f %1").arg(node.y(), 0, 'f', 1);
        davinci.drawText(leftDrawingRect(m_distTop), Qt::AlignRight, timeText);

        // yMin
        node = convertCanvasToTime(QPoint(m_distLeft, height()-1 - m_distBottom));
        timeText = QString("f %1").arg(node.y(), 0, 'f', 1);
        davinci.drawText(leftDrawingRect(height()-m_distBottom-8), Qt::AlignRight, timeText);

        // xMin
        node = convertCanvasToTime(QPoint(m_distLeft, height()-1 - m_distBottom));
        timeText = QString("f %1").arg(node.x(), 0, 'f', 1);
        davinci.drawText(bottomDrawingRect(m_distLeft+8), Qt::AlignRight, timeText);

        // xMax
        node = convertCanvasToTime(QPoint(width()-1 - m_distRight, height()-1 - m_distBottom));
        timeText = QString("f %1").arg(node.x(), 0, 'f', 1);
        davinci.drawText(bottomDrawingRect(width()-1-m_distRight), Qt::AlignRight, timeText);


    }
    int bottom = height()-1 - m_distBottom;
    davinci.drawLine(m_distLeft, bottom, width()-1 - m_distRight, bottom);
    davinci.drawLine(m_distLeft, bottom, m_distLeft, m_distTop);

    // Shutter Lengths (for motion blur)
    davinci.setRenderHint(QPainter::Antialiasing, false);
    const Node_sV *leftNode = NULL;
    const Node_sV *rightNode = NULL;
    const float outFps = m_project->preferences()->canvas_xAxisFPS().fps();
    const float sourceFps = m_project->frameSource()->fps()->fps();
    for (int i = 0; i < m_nodes->size(); i++) {
        rightNode = &m_nodes->at(i);
        QPoint p = convertTimeToCanvas(*rightNode);

        if (leftNode != NULL) {
            ShutterFunction_sV *shutterFunction = m_project->shutterFunctions()->function(leftNode->shutterFunctionID());
            if (shutterFunction != NULL) {

                QPoint pp = convertTimeToCanvas(*leftNode);
                for (int x = pp.x(); x < p.x(); x++) {
                    qreal progressOnCurve = ((qreal)x - pp.x()) / (p.x() - pp.x());

                    QPointF time;
                    if (leftNode->rightCurveType() == CurveType_Bezier && rightNode->leftCurveType() == CurveType_Bezier) {
                        time = BezierTools_sV::interpolateAtX(convertCanvasToTime(QPoint(x, 0)).x(),
                                    leftNode->toQPointF(), leftNode->toQPointF()+leftNode->rightNodeHandle(),
                                    rightNode->toQPointF()+rightNode->leftNodeHandle(), rightNode->toQPointF());
                    } else {
                        time = leftNode->toQPointF() + (rightNode->toQPointF() - leftNode->toQPointF()) * progressOnCurve;
                    }

                    qreal outTime = time.x();
                    qreal sourceTime = time.y();
                    qreal sourceFrame = sourceTime * sourceFps;

                    float dy;
                    if (outTime + 1/outFps <= m_nodes->endTime()) {
                        dy = m_nodes->sourceTime(outTime + 1/outFps) - sourceTime;
                    } else {
                        dy = sourceTime - m_nodes->sourceTime(outTime - 1/outFps);
                    }
                    float shutter = shutterFunction->evaluate(
                        progressOnCurve, // x on [0,1]
                        outTime, // t
                        outFps, // FPS
                        sourceFrame, // y
                        dy // dy to next frame
                        );
                    QPoint sourceShutterTimeStart = QPoint(x, convertTimeToCanvas(time).y());
                    QPoint sourceShutterTimeEnd = QPoint(x, convertTimeToCanvas(time + QPointF(0, shutter * outFps/sourceFps)).y());
                    if (shutter > 0) {
                        davinci.setPen(shutterRegionCol);
                        davinci.drawLine(sourceShutterTimeStart, sourceShutterTimeEnd);
                    }
                    davinci.setPen(shutterRegionBoundCol);
                    davinci.drawPoint(x, sourceShutterTimeEnd.y() - 1);
                }

            }
        }

        leftNode = &m_nodes->at(i);
    }

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
            if (m_project->nodes()->segments()->at(i-1).selected()) {
                davinci.setPen(selectedLineCol);
            } else {
                davinci.setPen(lineCol);
            }
            if (prev->rightCurveType() == CurveType_Bezier && curr->leftCurveType() == CurveType_Bezier) {
                QPainterPath path;
                path.moveTo(convertTimeToCanvas(*prev));
                path.cubicTo(
                            convertTimeToCanvas(prev->toQPointF() + prev->rightNodeHandle()),
                            convertTimeToCanvas(curr->toQPointF() + curr->leftNodeHandle()),
                            convertTimeToCanvas(*curr));
                davinci.drawPath(path);
#ifdef VALIDATE_BEZIER
                for (int x = convertTimeToCanvas(*prev).x(); x < p.x(); x++) {
                    QPointF py = BezierTools_sV::interpolateAtX(convertCanvasToTime(QPoint(x, 0)).x(),
                                                   prev->toQPointF(), prev->toQPointF()+prev->rightNodeHandle(),
                                                   curr->toQPointF()+curr->leftNodeHandle(), curr->toQPointF());
                    qreal y = convertTimeToCanvas(py).y();
//                    qDebug() << convertCanvasToTime(QPoint(x, 0)).x() << ": " << x << y;
                    davinci.drawPoint(x, y);
                }
#endif
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
        MainWindow *mw;
        if ((mw = dynamic_cast<MainWindow*>(parentWidget())) != NULL) {
            mw->displayHelp(davinci);
        } else {
            qDebug() << "Cannot show help; wrong parent widget?";
            Q_ASSERT(false);
        }
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
    davinci.drawImage(r - dR, t, QImage(":icons/iconSel.png").scaled(16, 16));
    dR += d+w;

    davinci.setOpacity(.5 + ((m_mode == ToolMode_Move) ? .5 : 0));
    davinci.drawImage(r - dR, t, QImage(":icons/iconMov.png").scaled(16, 16));

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
    m_states.initial_t0 = m_t0;

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
#ifdef DEBUG_C
                qDebug() << "Moving handle" << handle << " of node " << handle->parentNode()
                         << QString(" (%1)").arg(index);
                qDebug() << "Parent node x: " << handle->parentNode()->x();
                qDebug() << "Handle x: " << handle->x();
#endif

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
                        //qDebug() << "move selected";
                        bool snap = (e->modifiers() & Qt::ShiftModifier);
                        //TODO qDebug() << "is snap ? " << snap;
                        m_nodes->moveSelected(diff,snap);
                    }
                }
                m_states.nodesMoved = true;
            } else {
                // Cannot move this object, so move the canvas instead.
//                if (m_states.initialContextObject != NULL) {
//                    qDebug() << "Trying to move " << typeid(*m_states.initialContextObject).name() << ": Not supported yet!";
//                }

                m_t0 = m_states.initial_t0 - diff;
                if (m_t0.y() < 0) { m_t0.setY(0); }
                if (m_t0.x() < 0) { m_t0.setX(0); }
                if (m_t0.y() > m_tmax.y()) { m_t0.setY(m_tmax.y()); }

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

void Canvas::mouseReleaseEvent(QMouseEvent *event)
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
                                // check snap to grid
                                //qDebug()<< event->modifiers();
                                bool snap = (event->modifiers() & Qt::ShiftModifier);
                                //qDebug() << "snap to frame ? " << snap;
                                
                                Node_sV p = convertCanvasToTime(m_states.initialMousePos,snap);
                                //qDebug() << "adding node";
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
#if QT_VERSION < 0x040700
    } else if (m_states.initialButtons.testFlag(Qt::RightButton) || m_states.initialButtons.testFlag(Qt::MidButton)) {
#else
    } else if (m_states.initialButtons.testFlag(Qt::RightButton) || m_states.initialButtons.testFlag(Qt::MiddleButton)) {
#endif
        QList<NodeList_sV::PointerWithDistance> nearObjects = m_project->objectsNear(convertCanvasToTime(m_states.initialMousePos).toQPointF(),  delta(10));
        
#if _NEARBY_DBG
        qDebug() << "Nearby objects:";
        for (int i = 0; i < nearObjects.size(); i++) {
            qDebug() << typeid(*(nearObjects.at(i).ptr)).name() << " at distance " << nearObjects.at(i).dist;
        }
#endif
    }
    
}

void Canvas::contextMenuEvent(QContextMenuEvent *e)
{
    qDebug() << "Context menu requested";
    m_states.contextmenuMouseTime = convertCanvasToTime(e->pos()).toQPointF();

    QMenu menu;
    QMenu speedMenu(trUtf8("Segment replay &speed …"), &menu);

    const CanvasObject_sV *obj = objectAt(e->pos(), m_states.prevModifiers);

    if (dynamic_cast<const Node_sV*>(obj)) {
        Node_sV *node = (Node_sV *) obj;
        m_toDeleteNode.objectPointer = node;
        m_toSnapInNode.objectPointer = node;

        int nodeIndex = m_nodes->indexOf(node);

        menu.addAction(QString(tr("Node %1")).arg(nodeIndex))->setEnabled(false);
        menu.addAction(m_aDeleteNode);
//        menu.addAction(m_aSnapInNode); // \todo Activate Snap in
        menu.addSeparator()->setText(tr("Handle actions"));
        menu.addAction(m_aResetLeftHandle);
        menu.addAction(m_aResetRightHandle);

    } else if (dynamic_cast<const Segment_sV*>(obj) != NULL) {
        const Segment_sV* segment = (const Segment_sV*) obj;
        int leftNode = segment->leftNodeIndex();

        menu.addAction(QString(tr("Segment between node %1 and %2")).arg(leftNode).arg(leftNode+1))->setEnabled(false);
        menu.addAction(m_aLinear);
        menu.addAction(m_aBezier);
        menu.addAction(m_aShutterFunction);

        speedMenu.addAction(m_aCustomSpeed);
        std::vector<QAction*>::iterator it = m_aSpeeds.begin();
        while (it != m_aSpeeds.end()) {
            speedMenu.addAction(*it);
            it++;
        }
        menu.addMenu(&speedMenu);

    } else if (dynamic_cast<const Tag_sV*>(obj) != NULL) {
        Tag_sV* tag = (Tag_sV*) obj;
        m_toDeleteTag.objectPointer = tag;
        m_toRenameTag.objectPointer = tag;
        m_toSetTagTime.objectPointer = tag;

        menu.addAction(QString(tr("Tag %1")).arg(tag->description()));
        menu.addAction(m_aDeleteTag);
        menu.addAction(m_aRenameTag);
        menu.addAction(m_aSetTagTime);

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

void Canvas::keyPressEvent(QKeyEvent *event)
{
	if (dynamic_cast<const Node_sV*>(m_states.initialContextObject) != NULL) {
        const Node_sV *node = (const Node_sV*) m_states.initialContextObject;
        
        //qDebug() << "node : " << node->x() << "," << node->y();
        qDebug() << "canvas node : " << convertTimeToCanvas(*node);
        //qDebug() << "mouse " << m_states.prevMousePos << " vs " << m_states.initialMousePos;
        if (!m_states.nodesMoved) {
            qDebug() << "should be Moving node " << node;
        	Node_sV diff;
            
            switch (event->key()) {
                case Qt::Key_Up:
                    //qDebug() << "key up";
                    diff = convertCanvasToTime(QPoint(0,-1))-convertCanvasToTime(QPoint(0,0));
                    break;
                case Qt::Key_Down:
                    //qDebug() << "key down";
                    diff = convertCanvasToTime(QPoint(0,1))-convertCanvasToTime(QPoint(0,0));
                    break;
                case Qt::Key_Right:
                    //qDebug() << "key right";
                    diff = convertCanvasToTime(QPoint(1,0))-convertCanvasToTime(QPoint(0,0));
                    break;
                case Qt::Key_Left:
                    //qDebug() << "key left";
                    diff = convertCanvasToTime(QPoint(-1,0))-convertCanvasToTime(QPoint(0,0));
                    break;
            }
            //qDebug() << "moving of " << diff;
            m_nodes->moveSelected(diff);
            //TODO: update other windows ?
            //TODO: confirm move ?
            // from mouserelease
            
             /*if (m_states.countsAsMove()) */{
                m_nodes->confirmMove();
                //qDebug() << "key Move confirmed.";
                emit nodesChanged();
                
                repaint();
            }                        
#if 1
            // from mouse move ?
            // Emit the source time at the mouse position
            emit signalMouseInputTimeChanged(node->y()
                                             * m_project->frameSource()->fps()->fps()
                                             );
            
            //TODO: get right time !
            // Emit the source time at the intersection of the out time and the curve
            qreal timeOut = node->x();
            if (m_nodes->size() > 1 && m_nodes->startTime() <= timeOut && timeOut <= m_nodes->endTime()) {
                
#ifdef DEBUG_C
                std::cout.precision(32);
                std::cout << "start: " << m_nodes->startTime() << ", out: " << timeOut << ", end: " << m_nodes->endTime() << std::endl;
#endif
                
                if (m_nodes->find(timeOut) >= 0) {
                    emit signalMouseCurveSrcTimeChanged(
                                                        timeOut/*m_nodes->sourceTime(timeOut)*/
                                                        * m_project->frameSource()->fps()->fps());
                }
            }
#endif // mouse ?
            
                     
        }
        //event->ignore();
	}
	QWidget::keyPressEvent(event);
}

void Canvas::wheelEvent(QWheelEvent *e)
{
    // Mouse wheel movement in degrees
    int deg = e->delta()/8;

    if (e->modifiers().testFlag(Qt::ControlModifier)) {
        zoom(deg > 0, e->pos());
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
void Canvas::zoom(bool in, QPoint pos)
{
    Node_sV n0 = convertCanvasToTime(pos);

    // Update the line resolution
    if (in) {
        m_secResX *= ZOOM_FACTOR;
    } else {
        m_secResX /= ZOOM_FACTOR;
    }
    if (m_secResX < .05) { m_secResX = .05; }
    // Y resolution is the same as X resolution (at least at the moment)
    m_secResY = m_secResX;
//  qDebug() << "Resolution: " << m_secResX;

    // Adjust t0 such that the mouse points to the same time as before
    Node_sV nDiff = convertCanvasToTime(pos) - convertCanvasToTime(QPoint(m_distLeft, height()-1-m_distBottom));
    m_t0 = n0 - nDiff;
    if (m_t0.x() < 0) { m_t0.setX(0); }
    if (m_t0.y() < 0) { m_t0.setY(0); }

    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);
    Q_ASSERT(m_t0.x() >= 0);
    Q_ASSERT(m_t0.y() >= 0);
    repaint();
}
void Canvas::slotZoomIn()
{
    zoom(true, QCursor::pos());
}
void Canvas::slotZoomOut()
{
    zoom(false, QCursor::pos());
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

Node_sV Canvas::convertCanvasToTime(const QPoint &p, bool snap) const
{
    Q_ASSERT(m_secResX > 0);
    Q_ASSERT(m_secResY > 0);

    QPointF tDelta = convertDistanceToTime(QPoint(
                                                       p.x()-m_distLeft,
                                                       height()-1 - m_distBottom - p.y()
                                                       ));
    QPointF tFinal = tDelta + m_t0.toQPointF();
    //qDebug() << "convert: " << tFinal;
    if (snap) {
        tFinal.setX(int(tFinal.x()));
    }
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




void Canvas::slotRunAction(QObject *o)
{
    TransferObject *to = (TransferObject*) o;

    qDebug() << "Desired action: " << toString(to->reason);

    if (dynamic_cast<Tag_sV*>(to->objectPointer) != NULL) {

        /// Tag actions ///

        Tag_sV* tag = (Tag_sV*) to->objectPointer;
        qDebug() << " ... on Tag " << tag->description();
        switch (to->reason) {
        case TransferObject::ACTION_DELETE:
            for (int i = 0; i < m_tags->size(); ++i) {
                if (&m_tags->at(i) == tag) {
                    qDebug() << "Tag found, removing: " << m_tags->at(i).description();
                    m_tags->removeAt(i);
                    break;
                }
            }
            break;
        case TransferObject::ACTION_RENAME:
        {
            bool ok;
            QString newName = QInputDialog::getText(this, tr("New tag name"), tr("Tag:"), QLineEdit::Normal, tag->description(), &ok, 0, Qt::ImhNone );
            if (ok) {
                tag->setDescription(newName);
            }
            break;
        }
        case TransferObject::ACTION_SETTIME:
        {
            bool ok;
            double d = QInputDialog::getDouble(this, tr("New tag time"), tr("Time:"), tag->time(), 0, 424242, 5, &ok);
            if (ok) {
                tag->setTime(d);
            }
            break;
        }
        default:
            qDebug() << "Unknown action on Tag: " << toString(to->reason);
            Q_ASSERT(false);
            break;
        }
    } else if (dynamic_cast<Node_sV*>(to->objectPointer)) {

        /// Node actions ///

        Node_sV const* node = (Node_sV const*) to->objectPointer;
        qDebug() << " ... on node " << *node;
        switch (to->reason) {
        case TransferObject::ACTION_DELETE:
            m_nodes->deleteNode(m_nodes->indexOf(node));
            break;
        default:
            qDebug() << "Unknown action on Node: " << toString(to->reason);
            Q_ASSERT(false);
            break;
        }
    }

    repaint();
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

/**
 *
 */
void Canvas::setCurveSpeed(double speed)
{
    QString message;
    
    qDebug() << "Setting curve to " << speed << "x speed.";
    int errcode = m_nodes->setSpeed(convertCanvasToTime(m_states.prevMousePos).x(), speed);
    //TODO: should find a better way ... (try/catch ?)
    switch (errcode) {
        case -1:
            message = tr("%1 x speed would shoot over maximum time. Correcting.").arg(speed);
            QMessageBox(QMessageBox::Warning, tr("Warning"), message, QMessageBox::Ok).exec();
            break;
        case -2:
            message = tr("%1 x speed goes below 0. Correcting.").arg(speed);
            QMessageBox(QMessageBox::Warning, tr("Warning"), message, QMessageBox::Ok).exec();
            break;
        case -3:
            message = tr("New node would be too close, not adding it.");
            QMessageBox(QMessageBox::Warning, tr("Warning"), message, QMessageBox::Ok).exec();
            break;
        case -4 :
            message = tr("Outside segment.");
            QMessageBox(QMessageBox::Warning, tr("Warning"), message, QMessageBox::Ok).exec();
            break;
    }
    emit nodesChanged();
    repaint();
}


void Canvas::slotSetSpeed()
{
    bool ok = true;

    double d = m_settings.value("canvas/replaySpeed", 1.0).toDouble();
    qDebug() << "Getting: " << d;
    d = QInputDialog::getDouble(this, tr("Replay speed for current segment"), tr("Speed:"), d, -1000, 1000, 3, &ok);
    if (ok) {
        setCurveSpeed(d);
        m_settings.setValue("canvas/replaySpeed", d);
        qDebug() << "Setting: " << d;
    }
}
void Canvas::slotSetSpeed(QString s)
{
    bool ok = true;
    double d = s.toDouble(&ok);
    if (ok) {
        setCurveSpeed(d);
    } else {
        qDebug() << "Not ok: " << s;
    }
}
void Canvas::slotSetShutterFunction()
{
    int left = m_nodes->find(m_states.contextmenuMouseTime.x());
    if (left == m_nodes->size()-1) {
        left = m_nodes->size()-2;
    }

    if (m_shutterFunctionDialog == NULL) {
        m_shutterFunctionDialog = new ShutterFunctionDialog(m_project, this);
        connect(this, SIGNAL(nodesChanged()), m_shutterFunctionDialog, SLOT(slotNodesUpdated()));
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

QString toString(TransferObject::Reason reason)
{
    switch (reason) {
    case TransferObject::ACTION_DELETE :
        return "Delete";
    case TransferObject::ACTION_SNAPIN :
        return "Snap in";
    case TransferObject::ACTION_RENAME :
        return "Rename";
    case TransferObject::ACTION_SETTIME :
        return "Set time";
    default :
        Q_ASSERT(false);
        return "Unknown action";
    }
}
