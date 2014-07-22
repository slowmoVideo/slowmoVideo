/*
 * 2014 Valery Brasseur <vbrasseur@gmail.com>
 */

#include "renderTask_sV.h"
#include "abstractRenderTarget_sV.h"
#include "emptyFrameSource_sV.h"

#include <QCoreApplication>
#include <QImage>
#include <QMetaObject>
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>
#include "project_sV.h"
#include "nodeList_sV.h"
#include "../lib/defs_sV.hpp"

RenderTask_sV::RenderTask_sV(Project_sV *project) :
m_project(project),
m_renderTarget(NULL),
m_renderTimeElapsed(0),
m_stopRendering(false),
m_prevTime(-1)
{
    m_timeStart = m_project->nodes()->startTime();
    m_timeEnd = m_project->nodes()->endTime();
    
    m_nextFrameTime = m_project->nodes()->startTime();
    
    _working =false;
    
}

//TODO:
RenderTask_sV::~RenderTask_sV()
{
    if (m_renderTarget != NULL) { delete m_renderTarget; }
}

void RenderTask_sV::requestWork()
{
    mutex.lock();
    _working = true;
    m_stopRendering = false;
    qDebug()<<"rendering worker start in Thread "<<thread()->currentThreadId();
    mutex.unlock();
    
    emit workFlowRequested();
}

#pragma mark - progress

/**
 *  update progress dialog from outside
 */
void RenderTask_sV::updateProgress()
{
	qDebug() << "updateProgress call";
    emit signalTaskProgress( 50 );
}

#pragma mark - set/get task

void RenderTask_sV::slotStopRendering()
{
	//qDebug()<<"abort rendering required in Thread "<<thread()->currentThreadId();
    mutex.lock();
    if (_working) {
        m_stopRendering = true;
        qDebug()<<"rendering aborting in Thread "<<thread()->currentThreadId();
    }
    mutex.unlock();
}

void RenderTask_sV::setRenderTarget(AbstractRenderTarget_sV *renderTarget)
{
    Q_ASSERT(renderTarget != NULL);
    
    if (m_renderTarget != NULL && m_renderTarget != renderTarget) {
        delete m_renderTarget;
    }
    m_renderTarget = renderTarget;
}

void RenderTask_sV::setTimeRange(qreal start, qreal end)
{
    Q_ASSERT(start <= end);
    Q_ASSERT(start >= m_project->nodes()->startTime());
    Q_ASSERT(end <= m_project->nodes()->endTime());
    
    m_timeStart = start;
    m_timeEnd = end;
}

void RenderTask_sV::setTimeRange(QString start, QString end)
{
    Q_ASSERT(m_prefs.fpsSetByUser());
    m_timeStart = m_project->toOutTime(start, m_prefs.fps());
    m_timeEnd = m_project->toOutTime(end, m_prefs.fps());
    Q_ASSERT(m_timeStart < m_timeEnd);
}

QSize RenderTask_sV::resolution()
{
    return const_cast<Project_sV*>(m_project)->frameSource()->frameAt(0, m_prefs.size).size();
}


#pragma mark - rendering

/**
 *  this is the real workhorse.
 * maybe we should not call this directly, but instead from doWork ?
 */
void RenderTask_sV::slotContinueRendering()
{
    qDebug()<<"Starting rendering process in Thread "<<thread()->currentThreadId();   
    /* real workhorse */
    emit signalNewTask(trUtf8("Rendering Slow-Mo …"), int(m_prefs.fps().fps() * (m_timeEnd-m_timeStart)));
        
    //TODO: initialize
    m_stopwatch.start();
    
    m_nextFrameTime=m_timeStart;
    
    int framesBefore;
    qreal snapped = m_project->snapToOutFrame(m_nextFrameTime, false, m_prefs.fps(), &framesBefore);
    qDebug() << "Frame snapping in from " << m_nextFrameTime << " to " << snapped;
    m_nextFrameTime = snapped;
    
    Q_ASSERT(int((m_nextFrameTime - m_project->nodes()->startTime()) * m_prefs.fps().fps() + .5) == framesBefore);
    
    try {
    	m_renderTarget->openRenderTarget();
    } catch (Error_sV &err) {
            m_stopRendering = true;
            emit signalRenderingAborted(tr("Rendering aborted.") + " " + err.message());
            return;
    }
    
    // render loop
    // TODO: add more threading here
    while(m_nextFrameTime<m_timeEnd) {
    	    QCoreApplication::processEvents();
    	    
        // Checks if the process should be aborted
        mutex.lock();
        bool abort = m_stopRendering;
        mutex.unlock();
        
        if (abort) {
        	// user stop the process
            qDebug()<<"Aborting Rendering process in Thread "<<thread()->currentThreadId();
            emit signalRenderingStopped(QTime().addMSecs(m_renderTimeElapsed).toString("hh:mm:ss"));
        	qDebug() << "Rendering stopped after " << QTime().addMSecs(m_renderTimeElapsed).toString("hh:mm:ss");
            break;
        }
        
        // do the work
        int outputFrame = (m_nextFrameTime - m_project->nodes()->startTime()) * m_prefs.fps().fps() + .5;
        qreal srcTime = m_project->nodes()->sourceTime(m_nextFrameTime);
        
        qDebug() << "Rendering frame number " << outputFrame << " @" << m_nextFrameTime << " from source time " << srcTime;
        emit signalItemDesc(tr("Rendering frame %1 @ %2 s  from input position: %3 s (frame %4)")
                            .arg( QString::number(outputFrame),QString::number(m_nextFrameTime),
                                  QString::number(srcTime),
                                  QString::number(srcTime*m_project->frameSource()->fps()->fps())
                             ) );
           
         try {
                QImage rendered = m_project->render(m_nextFrameTime, m_prefs);

                m_renderTarget->slotConsumeFrame(rendered, outputFrame);
                m_nextFrameTime = m_nextFrameTime + 1/m_prefs.fps().fps();

                emit signalTaskProgress( (m_nextFrameTime-m_timeStart) * m_prefs.fps().fps() );
                
            } catch (FlowBuildingError &err) {
                m_stopRendering = true;
                emit signalRenderingAborted(err.message());
            } catch (InterpolationError &err) {
                emit signalItemDesc(err.message());
            }
        
        
    } /* while */
    
    
    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();
    
    //TODO: closing rendering project
    qDebug() << "Rendering : exporting";
    emit signalItemDesc(tr("Rendering : exporting"));
    m_renderTarget->closeRenderTarget();
    m_renderTimeElapsed += m_stopwatch.elapsed();
    emit signalRenderingFinished(QTime().addMSecs(m_renderTimeElapsed).toString("hh:mm:ss"));
    qDebug() << "Rendering stopped after " << QTime().addMSecs(m_renderTimeElapsed).toString("hh:mm:ss");
    
    qDebug()<<"Rendering process finished in Thread "<<thread()->currentThreadId();
}
