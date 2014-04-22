/*
 * precalculate optical flow
 */

#include "work_flow.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

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
    qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
    mutex.unlock();
    
    emit workRequested();
}

void WorkerFlow::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
    mutex.unlock();
}

void setFrameSize(FrameSize _frameSize)
{
    frameSize = _frameSize;
}

void WorkerFlow::doWorkFlow()
{
    qDebug()<<"Starting worker process in Thread "<<thread()->currentThreadId();
    
    while(1)) {
        
        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();
        
        if (abort) {
            qDebug()<<"Aborting worker process in Thread "<<thread()->currentThreadId();
            break;
        }
        
        //flowSource()->buildFlowForwardCache(frameSize);
        // This will stupidly wait 1 sec doing nothing...
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();
        
        // Once we're done waiting, value is updated
        emit valueChanged(QString::number(i));
    }
    
    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();
    
    qDebug()<<"Worker process finished in Thread "<<thread()->currentThreadId();
    
    //Once 60 sec passed, the finished signal is sent
    emit finished();
}
