/*
 * precalculate optical flow
 */

#include "flowSourceOpenCV_sV.h"
#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "../lib/flowRW_sV.h"
#include "../lib/flowField_sV.h"

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


#include "work_flow.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

using namespace cv;

extern void drawOptFlowMap(const Mat& flow, int step,
                         double, const Scalar& color, std::string flowname );



WorkerFlow::WorkerFlow(QObject *parent) :
QObject(parent)
{
    _working =false;
    _abort = false;
    
}

void WorkerFlow::requestWork()
{
    mutex.lock();
    _working = true;
    _abort = false;
    qDebug()<<"OpticalFlow worker start in Thread "<<thread()->currentThreadId();
    mutex.unlock();
    
    emit workFlowRequested();
}

void WorkerFlow::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
        qDebug()<<"OpticalFlow worker aborting in Thread "<<thread()->currentThreadId();
    }
    mutex.unlock();
}


//TODO: should get an object Flow...
void WorkerFlow::setFrameSize(FrameSize _frameSize)
{
    frameSize = _frameSize;
}

void WorkerFlow::setProject(Project_sV *_project)
{
    project = _project;
}


void WorkerFlow::setFlowSource(AbstractFlowSource_sV* _flowsource)
{
    flowSource = _flowsource;
}

#if 0
//TODO: better function for this ... (
// common with flowSourceOpenCv ?
const QString WorkerFlow::flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const
{
    QDir dir;
    if (frameSize == FrameSize_Orig) {
        dir = m_dirFlowOrig;
    } else {
        dir = m_dirFlowSmall;
    }
    QString direction;
    if (leftFrame < rightFrame) {
        direction = "forward";
    } else {
        direction = "backward";
    }
    
    return dir.absoluteFilePath(QString("ocv-%1-%2-%3.sVflow").arg(direction).arg(leftFrame).arg(rightFrame));
}
#endif

//TODO: call abstractflow source method
void WorkerFlow::doWorkFlow()
{
    qDebug()<<"Starting OpticalFlow process in Thread "<<thread()->currentThreadId();
    
    /* out of loop work */
    int lastFrame = project->frameSource()->framesCount();
    int frame = 0;
    Mat prevgray, gray, flow;
    
    qDebug() << "Pre Building forward flow for Size: " << frameSize;
    
#if 0
    //TODO:
    m_dirFlowSmall = project->getDirectory("cache/oFlowSmall");
    m_dirFlowOrig = project->getDirectory("cache/oFlowOrig");
#endif

    // load first frame
    QString prevpath = project->frameSource()->framePath(frame, frameSize);
    prevgray = imread(prevpath.toStdString(), 0);
    
    // TODO: need sliders for all these parameters
    const float pyrScale = 0.5; // classical pyr
    const float levels = 3;
    const float winsize = 15;
    const float iterations = 8;
    const float polyN = 5;
    const float polySigma = 1.2;
    int flags = 0;
    
    /* real workhorse */
    for(frame=0;frame<lastFrame;frame++) {
        
        
        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();
        
        if (abort) {
            qDebug()<<"Aborting OpticalFlow process in Thread "<<thread()->currentThreadId();
            break;
        }
        
        QString flowFileName(flowSource->flowPath(frame, frame+1, frameSize));
        
        qDebug() << "Building flow for left frame " << frame << " to right frame " << frame+1 << "; Size: " << frameSize;
        /// \todo Check if size is equal
        if (!QFile(flowFileName).exists()) {
            
            //QTime time;
            //time.start();
            
            QString prevpath = project->frameSource()->framePath(frame, frameSize);
            QString path = project->frameSource()->framePath(frame+1, frameSize);
            
            gray = imread(path.toStdString(), 0);
            
            // use previous flow info
            //if (frame!=0)
            //    flags |= OPTFLOW_USE_INITIAL_FLOW;
            
            calcOpticalFlowFarneback(
                                     prevgray, gray,
                                     flow,
                                     pyrScale, //0.5,
                                     levels, //3,
                                     winsize, //15,
                                     iterations, //3,
                                     polyN, //5,
                                     polySigma, //1.2,
                                     flags //0 OPTFLOW_USE_INITIAL_FLOW
                                     );
            // save result
            drawOptFlowMap(flow, 1, 1.5, CV_RGB(0, 255, 0), flowFileName.toStdString());
            std::swap(prevgray, gray);
            qDebug() << "Optical flow built for " << flowFileName;
            
        } else {
            qDebug().nospace() << "Re-using existing flow image for left frame " << frame << " to right frame " << frame+1 << ": " << flowFileName;
        }
        //qDebug() << "Optical flow built for " << flowFileName << " in " << time.elapsed() << " ms.";
        // Once we're done waiting, value is updated
        emit valueChanged(QString::number(frame));
        
    } /* for */
    
    
    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();
    
    qDebug()<<"OpticalFlow process finished in Thread "<<thread()->currentThreadId();
    
    // the finished signal is sent
    emit finished();
}
