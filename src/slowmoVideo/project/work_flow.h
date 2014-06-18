/*
 precalculate optical flow
* 2014 Valery Brasseur <vbrasseur@gmail.com>
*/

#ifndef WORKERFLOW_H
#define WORKERFLOW_H

#include "project_sV.h"

#include <QObject>
#include <QMutex>

class WorkerFlow : public QObject
{
    Q_OBJECT
    
public:
    explicit WorkerFlow(QObject *parent = 0);
    /**
     * Requests the process to start
     *
     * It is thread safe as it uses #mutex to protect access to #_working variable.
     */
    void requestWork();
    /**
     *  Requests the process to abort
     *
     * It is thread safe as it uses #mutex to protect access to #_abort variable.
     */
    void abort();
    
    /*
     * set running size
     */
    void setFrameSize(FrameSize _frameSize);
    void setProject(Project_sV *_project);
    
    void setFlowSource(AbstractFlowSource_sV* _flowsource);

	void setDirection(int _forward) { forward = _forward;};
	
private:
    /**
     *  Process is aborted when true
     */
    bool _abort;
    /**
     *  true when Worker is doing work
     */
    bool _working;
    /**
     * Protects access to #_abort
     */
    QMutex mutex;
    
    /**
     * which source flow
     */
    AbstractFlowSource_sV* flowSource;
    
    /**
     * which size do we create
     */
    FrameSize frameSize;
    
    /*
     * which project is concern
     */
    Project_sV *project;
    
    int forward;
#if 0
    const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const;
    
    QDir m_dirFlowSmall;
    QDir m_dirFlowOrig;
#endif
    

signals:
    /**
     * This signal is emitted when the Worker request to Work
     * requestWork()
     */
    void workFlowRequested();
    /**
     * This signal is emitted when counted value is changed (every sec)
     */
    void valueChanged(const QString &value);
    /**
     * This signal is emitted when process is finished (or being aborted)
     */
    void finished();
    
    public slots:
    /**
     * calculate optical flow
     *
     */
    void doWorkFlow();
};

#endif // WORKER_H

