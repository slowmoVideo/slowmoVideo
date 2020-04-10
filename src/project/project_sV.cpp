/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "project_sV.h"
#include "projectPreferences_sV.h"
#include "videoFrameSource_sV.h"
#include "emptyFrameSource_sV.h"
#include "flowSourceV3D_sV.h"
#include "flowSourceOpenCV_sV.h"
#include "interpolator_sV.h"
#include "motionBlur_sV.h"
#include "nodeList_sV.h"
#include "renderTask_sV.h"
#include "shutterFunction_sV.h"
#include "shutterFunctionList_sV.h"
#include "../lib/shutter_sV.h"
#include "../lib/interpolate_sV.h"
#include "../lib/flowRW_sV.h"
#include "../lib/flowField_sV.h"

#if 1
#include "work_flow.h"
#endif

#include <cmath>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QThread>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QTemporaryDir>
#endif

//#define DEBUG_P
#ifdef DEBUG_P
#include <iostream>
#endif


#define MIN_FRAME_DIST .001

Project_sV::Project_sV()
{

//TODO: scoping problem
    m_projDir = getDirectoryName();

    init();

    int tid;
    for(tid=0;tid<4;tid++) {
      worker[tid]=0;
      thread[tid]=0;
    }
}

Project_sV::Project_sV(QString projectDir) : m_projDir(projectDir)
{
    init();
}

QDir Project_sV::getDirectoryName()
{
    QDir dirName;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    //QTemporaryDir tempDir("slowmovideo");
    QTemporaryDir tempDir;; // use default
    if (tempDir.isValid())
        dirName = QDir(tempDir.path());
    else
#endif
        dirName = QDir::temp();

    return dirName;
}

void Project_sV::init()
{
    m_preferences = new ProjectPreferences_sV();
    m_frameSource = new EmptyFrameSource_sV(this);
    m_flowSource = 0; // leak ? new FlowSourceV3D_sV(this);
    m_motionBlur = new MotionBlur_sV(this);
    reloadFlowSource();
    m_tags = new QList<Tag_sV>();
    m_nodes = new NodeList_sV();
    m_shutterFunctions = new ShutterFunctionList_sV(m_nodes);
    m_renderTask = NULL;

    m_v3dFailCounter = 0;

    /* better here ? */
    int tid;
    for(tid=0;tid<4;tid++) {
      worker[tid]=0;
      thread[tid]=0;
    }
}

Project_sV::~Project_sV()
{
    int tid;
    for(tid=0;tid<4;tid++) {
      if (worker[tid]!=0) {
            worker[tid]->abort();
            thread[tid]->wait();

            //qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();

            delete worker[tid];
            delete thread[tid];
      }
    }

    delete m_renderTask;
    qDebug() << "delete pref";
    delete m_preferences;
    delete m_frameSource;
    delete m_flowSource;
    delete m_motionBlur;
    delete m_tags;
    delete m_nodes;
    delete m_shutterFunctions;
}

void Project_sV::reloadFlowSource()
{
    if (m_flowSource != 0)
        delete m_flowSource;

    QSettings m_settings;
    QString method = m_settings.value("preferences/flowMethod", "OpenCV-CPU").toString();
    if ("V3D" == method) {
        m_flowSource = new FlowSourceV3D_sV(this);
    } else {
        int algo = m_settings.value("preferences/preferredOpenCVAlgo", 0).toInt();
        int dev_index = m_settings.value("preferences/oclDriver", -1).toInt();
        if (method == "OpenCV-CPU") {
            dev_index = -1;
        }
        m_flowSource = new FlowSourceOpenCV_sV(this, algo, dev_index);
    }
}

void Project_sV::setupProjectDir()
{
    //qDebug() << "Project directory: " << m_projDir.absolutePath();

    m_frameSource->slotUpdateProjectDir();
    m_flowSource->slotUpdateProjectDir();
    m_motionBlur->slotUpdateProjectDir();
}

void Project_sV::setProjectDir(QString projectDir)
{
    m_projDir = projectDir;
    // Create directory if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }

    setupProjectDir();
}

void Project_sV::setProjectFilename(QString filename)
{
    m_projectFilename = filename;
}

QString Project_sV::projectFilename() const {
    return m_projectFilename;
}

void Project_sV::loadFrameSource(AbstractFrameSource_sV *frameSource)
{
    if (m_frameSource != NULL) {
        delete m_frameSource;
    }
    if (frameSource == NULL) {
        m_frameSource = new EmptyFrameSource_sV(this);
    } else {
        m_frameSource = frameSource;
    }
    m_nodes->setMaxY(m_frameSource->maxTime());
}


//TODO: remove this
void Project_sV::replaceRenderTask(RenderTask_sV *task)
{
    /*
    if (m_renderTask != NULL) {
        m_renderTask->slotStopRendering();
        m_renderTask->deleteLater();
        m_renderTask = NULL;
    }
     */
    m_renderTask = task;
}


const QDir Project_sV::getDirectory(const QString &name, bool createIfNotExists) const
{
    QDir dir(m_projDir.absolutePath() + "/" + name);
    if (createIfNotExists && !dir.exists()) {
        dir.mkpath(".");
    }
    return dir;
}

QImage Project_sV::render(qreal outTime, RenderPreferences_sV prefs)
{
    if (outTime < m_nodes->startTime() || outTime > m_nodes->endTime()) {
#ifdef DEBUG_P
        std::cout.precision(30);
        std::cout << m_nodes->startTime() << " -- " << outTime << " -- " << m_nodes->endTime() << std::endl;
        std::cout.flush();
#endif
        qDebug() << "Output time out of bounds";
        Q_ASSERT(false);
    }

    float sourceTime = m_nodes->sourceTime(outTime);
    if (sourceTime < 0) {
        sourceTime = 0;
    }
    if (sourceTime > m_frameSource->maxTime()) {
        sourceTime = m_frameSource->maxTime();
    }

    float sourceFrame = sourceTimeToFrame(sourceTime);

    int leftIndex = m_nodes->find(outTime);
    if (leftIndex < 0) {
        qDebug() << "left node is not here!";
        Q_ASSERT(false);
    }
    if (leftIndex == m_nodes->size()-1) {
        // outTime is at the very end of the node.
        // Take next to last node to still have a right node.
        leftIndex--;
    }

    const Node_sV *leftNode = &(*m_nodes)[leftIndex];
    const Node_sV *rightNode = &(*m_nodes)[leftIndex+1];

    ShutterFunction_sV *shutterFunction = m_shutterFunctions->function(leftNode->shutterFunctionID());

    if (shutterFunction != NULL) {
        float dy = 0;
        if (outTime+1/prefs.fps().fps() <= m_nodes->endTime()) {
            dy = m_nodes->sourceTime(outTime+1/prefs.fps().fps()) - sourceTime;
        } else {
            dy = sourceTime - m_nodes->sourceTime(outTime-1/prefs.fps().fps());
        }
        float replaySpeed = fabs(dy)*prefs.fps().fps();
        float shutter = shutterFunction->evaluate(
                    (outTime-leftNode->x())/(rightNode->x()-leftNode->x()), // x on [0,1]
                    outTime, // t
                    prefs.fps().fps(), // FPS
                    sourceFrame, // y
                    dy // dy to next frame
                    );
        qDebug() << "Shutter value for output time " << outTime << " is " << shutter;
        if (shutter > 0) {
            try {
                return m_motionBlur->blur(sourceFrame, sourceFrame+shutter*prefs.fps().fps(),
                                          replaySpeed,
                                          prefs);
            } catch (RangeTooSmallError_sV &err) {}
        }
    }
    return Interpolator_sV::interpolate(this, sourceFrame, prefs);
}

FlowField_sV* Project_sV::requestFlow(int leftFrame, int rightFrame, const FrameSize frameSize) throw(FlowBuildingError)
{
    Q_ASSERT(leftFrame < m_frameSource->framesCount());
    Q_ASSERT(rightFrame < m_frameSource->framesCount());
    if (dynamic_cast<EmptyFrameSource_sV*>(m_frameSource) == NULL) {

        FlowSourceV3D_sV *v3d;
        if ((v3d = dynamic_cast<FlowSourceV3D_sV*>(m_flowSource)) != NULL) {
            v3d->setLambda(m_preferences->flowV3DLambda());
            try {
                return m_flowSource->buildFlow(leftFrame, rightFrame, frameSize);
            } catch (FlowBuildingError err) {
                m_v3dFailCounter++;
                qDebug() << "Failed creating optical flow, falling back to OpenCV ...";
                qDebug() << "Failed attempts so far: " << m_v3dFailCounter;
                delete m_flowSource;
                QSettings m_settings;
                int algo = m_settings.value("preferences/preferredOpenCVAlgo", 0).toInt();
                int dev_index = m_settings.value("preferences/oclDriver", -1).toInt();
                QString method = m_settings.value("preferences/flowMethod", "OpenCV-CPU").toString();
                if (method == "OpenCV-CPU") {
                    dev_index = -1;
                }
                m_flowSource = new FlowSourceOpenCV_sV(this, algo, dev_index);
                return m_flowSource->buildFlow(leftFrame, rightFrame, frameSize);
            }
        }
        return m_flowSource->buildFlow(leftFrame, rightFrame, frameSize);
    } else {
        throw FlowBuildingError(tr("Empty frame source; Cannot build flow."));
    }
}

inline
qreal Project_sV::sourceTimeToFrame(qreal time) const
{
    Q_ASSERT(time >= 0);
    return time * m_frameSource->fps()->fps();
}

qreal Project_sV::snapToFrame(const qreal time, bool roundUp, const Fps_sV &fps, int *out_framesBeforeHere)
{
    Q_ASSERT(time >= 0);
    int frameCount = 0;
    double snapTime = 0;
    double frameLength;
    frameLength = 1.0/fps.fps();

    while (snapTime < time) {
        snapTime += frameLength;
        frameCount++;
    }

    // snapTime is now >= time

    if (!roundUp && snapTime != time) {
        snapTime -= frameLength;
        frameCount--;
    }

    if (out_framesBeforeHere != NULL) {
        *out_framesBeforeHere = frameCount;
    }

    return snapTime;
}
qreal Project_sV::snapToOutFrame(qreal time, bool roundUp, const Fps_sV &fps, int *out_framesBeforeHere) const
{
    if (time > m_nodes->endTime()) {
        time = m_nodes->endTime();
    }
    time -= m_nodes->startTime();
    if (time < 0) { time = 0; }
    float snapped = snapToFrame(time, roundUp, fps, out_framesBeforeHere) + m_nodes->startTime();
    return snapped;
}
qreal Project_sV::toOutTime(QString timeExpression, const Fps_sV &fps) const throw(Error_sV)
{
    if (m_nodes->size() < 2) {
        throw Error_sV(tr("Not enough nodes available in the project."));
    }

    // t:time l:label f:frame p:percent :start :end time
    bool ok = false;
    qreal time = 0;
    if (timeExpression.startsWith("t:")) {
        time = timeExpression.mid(2).toDouble(&ok);
        if (!ok) {
            throw Error_sV(tr("%1 is not a valid time. Format: t:123.45").arg(timeExpression));
        }
    } else if (timeExpression.startsWith("l:")) {
        QString label = timeExpression.mid(2);
        bool inputAxisFound = false;
        for (int i = 0; i < m_tags->size(); i++) {
            if (m_tags->at(i).description() == label) {
                if (m_tags->at(i).axis() == TagAxis_Output) {
                    time = m_tags->at(i).time();
                    ok = true;
                    break;
                } else {
                    inputAxisFound = true;
                }
            }
        }
        if (!ok) {
            if (inputAxisFound) {
                throw Error_sV(tr("%1 is an input label and not an output label and cannot be used for rendering.").arg(label));
            } else {
                throw Error_sV(tr("No label found for %1").arg(timeExpression));
            }
        }
    } else if (timeExpression.startsWith("f:")) {
        int frame = timeExpression.mid(2).toInt(&ok);
        if (ok) {
            time = frame / fps.fps();
        } else {
            throw Error_sV(tr("%1 is not a valid frame number. Format: f:1234").arg(timeExpression));
        }
    } else if (timeExpression.startsWith("p:")) {
        QString sPercent = timeExpression.mid(2).trimmed();
        if (sPercent.endsWith("%")) {
            sPercent.chop(1);
        }
        float percent = sPercent.toFloat(&ok);
        if (ok) {
            if (percent >= 0 && percent <= 100) {
                time = m_nodes->startTime() + m_nodes->totalTime()*percent/100;
            } else {
                throw Error_sV(tr("%1 is not a valid percentage number; must be between 0 and 100.").arg(percent));
            }
        } else {
            throw Error_sV(tr("%1 is not a valid percentage expression. Format: p:0% until p:100.0%").arg(timeExpression));
        }
    } else if (timeExpression.startsWith(":")) {
        if (":start" == timeExpression) {
            time = m_nodes->startTime();
        } else if (":end" == timeExpression) {
            time = m_nodes->endTime();
        } else {
            throw Error_sV(tr("%1 is not a valid position. Valid: :start and :end").arg(timeExpression));
        }
    } else {
        time = timeExpression.toDouble(&ok);
        if (!ok) {
            throw Error_sV(tr("Not a valid time format. Options:  t:1.25 or 1.25 (time),  f:1234 (frame),  "
                                   "l:slowdown (label),  p:42.42% (percentage),  :start and :end (project start/end)."));
        }
    }

    time = qMax(time, m_nodes->startTime());
    time = qMin(time, m_nodes->endTime());

    return time;
}

QList<NodeList_sV::PointerWithDistance> Project_sV::objectsNear(QPointF pos, qreal tmaxdist) const
{
    QList<NodeList_sV::PointerWithDistance> list = m_nodes->objectsNear(pos, tmaxdist);

    qreal dist;
    for (int i = 0; i < m_tags->size(); i++) {
        if (m_tags->at(i).axis() == TagAxis_Source) {
            dist = fabs(pos.y() - m_tags->at(i).time());
        } else {
            dist = fabs(pos.x() - m_tags->at(i).time());
        }
        if (dist <= tmaxdist) {
            list << NodeList_sV::PointerWithDistance(&m_tags->at(i), dist, NodeList_sV::PointerWithDistance::Tag);
        }
    }

    qSort(list);

    return list;
}

/**
 *  start an optical flow on a thread
 *
 *  @param threadid  the thread on which to run our flow
 *  @param frameSize size of frame for calcul (small/orig)
 *  @param direction flow direction
 */
void Project_sV::startFlow(int threadid,const FrameSize frameSize,int direction)
{
    thread[threadid] = new QThread();
    worker[threadid] = new WorkerFlow();

    // set on what to work ...
    worker[threadid]->setFrameSize(frameSize);
    worker[threadid]->setProject(this);
    worker[threadid]->setDirection(direction);
    worker[threadid]->setFlowSource(flowSource());

    worker[threadid]->moveToThread(thread[threadid]);
    //connect(worker, SIGNAL(valueChanged(QString)), ui->label, SLOT(setText(QString)));
    connect(worker[threadid], SIGNAL(workFlowRequested()), thread[threadid], SLOT(start()));
    connect(thread[threadid], SIGNAL(started()), worker[threadid], SLOT(doWorkFlow()));
    connect(worker[threadid], SIGNAL(finished()), thread[threadid], SLOT(quit()), Qt::DirectConnection);

    // let's start
    thread[threadid]->wait(); // If the thread is not running, this will immediately return.

    worker[threadid]->requestWork();
}

/**
 * prebuild the optical flow using threading
 */
void Project_sV::buildCacheFlowSource()
{

    QSettings settings;
    bool precalc = settings.value("preferences/precalcFlow", true).toBool();
    if (precalc && (m_flowSource != NULL)) {
#if 0
        qDebug() << "Creating cached FlowSources ";
        Q_ASSERT(m_flowSource != NULL);
        // TODO: test/check better place ?
        // we should do it for each size/each way
        // use threading here

        startFlow(0,FrameSize_Small,0);
        startFlow(1,FrameSize_Small,1);
        startFlow(2,FrameSize_Orig,0);
        startFlow(3,FrameSize_Orig,1);
#else
        qDebug() << "cache flow source disable";
#endif

    }

}

