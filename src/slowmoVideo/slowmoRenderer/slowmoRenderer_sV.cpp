/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "config.h"

#include "slowmoRenderer_sV.h"

#include "project/project_sV.h"
#include "project/projectPreferences_sV.h"
#include "project/xmlProjectRW_sV.h"
#include "project/renderTask_sV.h"
#include "project/imagesRenderTarget_sV.h"

#include "project/videoFrameSource_sV.h"
#include "project/emptyFrameSource_sV.h"
#include "project/imagesFrameSource_sV.h"

#ifdef USE_FFMPEG
#if 0
#include "project/new_videoRenderTarget.h"
#else
#include "project/exportVideoRenderTarget.h"
#endif
#else
#include "project/videoRenderTarget_sV.h"
#endif

#include "project/flowSourceV3D_sV.h"

#include <iostream>

Error::Error(std::string message) :
    // m_nodes->setMaxY(m_frameSource->maxTime());
    message(message) {}

SlowmoRenderer_sV::SlowmoRenderer_sV() :
    m_project(NULL),
    m_taskSize(0),
    m_lastProgress(0),
    m_start(":start"),
    m_end(":end"),
    m_renderTargetSet(false)
{
}

SlowmoRenderer_sV::~SlowmoRenderer_sV()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        m_project->getProjectDir().removeRecursively();
#else
#warning  removeRecursively not define in QT4
#endif

    delete m_project;
}

void SlowmoRenderer_sV::save(QString filename)
{
	XmlProjectRW_sV writer;
	writer.saveProject(m_project, filename);
}

void SlowmoRenderer_sV::load(QString filename) throw(Error)
{
    if (m_project != NULL) {
        delete m_project;
        m_project = NULL;
    }
    QString warning;
    try {
        Project_sV *proj = XmlProjectRW_sV::loadProject(QString(filename), &warning);

        if (warning.length() > 0) {
            std::cout << warning.toStdString() << std::endl;
        }

        m_project = proj;

        RenderTask_sV *task = new RenderTask_sV(m_project);
        m_project->replaceRenderTask(task);
        task->renderPreferences().setFps(24);
        task->setTimeRange(m_start, m_end);

        connect(m_project->renderTask(), SIGNAL(signalNewTask(QString,int)), this, SLOT(slotTaskSize(QString,int)));
        connect(m_project->renderTask(), SIGNAL(signalTaskProgress(int)), this, SLOT(slotProgressInfo(int)));
        connect(m_project->renderTask(), SIGNAL(signalRenderingAborted(QString)), this, SLOT(slotFinished(QString)));
        connect(m_project->renderTask(), SIGNAL(signalRenderingFinished(QString)), this, SLOT(slotFinished(QString)));
        connect(m_project->renderTask(), SIGNAL(signalRenderingStopped(QString)), this, SLOT(slotFinished(QString)));

    } catch (Error_sV &err) {
        throw Error(err.message().toStdString());
    }
}

void SlowmoRenderer_sV::create() throw(Error)
{
    std::cout << "Standalone Rendering." << std::endl;
    
    if (m_project != NULL) {
        delete m_project;
        m_project = NULL;
    }
    
    try {
 

        m_project = new Project_sV();
        
        
        RenderTask_sV *task = new RenderTask_sV(m_project);
        m_project->replaceRenderTask(task);
        task->renderPreferences().setFps(24);
        //task->setTimeRange(m_start, m_end);
        
        connect(m_project->renderTask(), SIGNAL(signalNewTask(QString,int)), this, SLOT(slotTaskSize(QString,int)));
        connect(m_project->renderTask(), SIGNAL(signalTaskProgress(int)), this, SLOT(slotProgressInfo(int)));
        connect(m_project->renderTask(), SIGNAL(signalRenderingAborted(QString)), this, SLOT(slotFinished(QString)));
        connect(m_project->renderTask(), SIGNAL(signalRenderingFinished(QString)), this, SLOT(slotFinished(QString)));
        connect(m_project->renderTask(), SIGNAL(signalRenderingStopped(QString)), this, SLOT(slotFinished(QString)));
        
    } catch (Error_sV &err) {
        throw Error(err.message().toStdString());
    }
}


void SlowmoRenderer_sV::setSpeed(double slowfactor)
{
        /* add a first (default) node */
        Node_sV snode;
        
        snode.setX(0.0);
        snode.setY(0.0);
        m_project->nodes()->add(snode);
        
        Node_sV enode;
       	// linear slope ? 
	// maybe should calc ?
	// need to check for video loaded ?
        enode.setY(m_project->frameSource()->maxTime());
        enode.setX((1/slowfactor)*m_project->frameSource()->maxTime());
        m_project->nodes()->add(enode);

	//m_project->nodes()->setSpeed(0,slowfactor);
	m_project->renderTask()->setTimeRange(m_start, m_end);
}

void SlowmoRenderer_sV::setTimeRange(QString start, QString end)
{
    m_start = start;
    m_end = end;
    m_project->renderTask()->setTimeRange(m_start, m_end);
}

void SlowmoRenderer_sV::setFps(double fps)
{
    m_project->renderTask()->renderPreferences().setFps(fps);
}

void SlowmoRenderer_sV::setInputTarget(QString inFilename)
{
    m_project->loadFrameSource(new VideoFrameSource_sV(m_project, inFilename));
    
    connect(m_project->frameSource(), SIGNAL(signalNextTask(QString,int)), this, SLOT(slotNewFrameSourceTask(QString,int)));
    connect(m_project->frameSource(), SIGNAL(signalAllTasksFinished()), this, SLOT(slotFrameSourceTasksFinished()));
    
    //m_project->frameSource()->initialize();
    m_project->frameSource()->loadOrigFrames();
    // m_nodes->setMaxY(m_frameSource->maxTime());
//   	std::cerr << "max time : " << m_project->frameSource()->maxTime() << std::endl; 
}
                                             
void SlowmoRenderer_sV::setVideoRenderTarget(QString filename, QString codec)
{
#ifdef USE_FFMPEG
#if 0
	#warning "using QTKit version"
    newVideoRenderTarget *vrt = new newVideoRenderTarget(m_project->renderTask()); 
#else
    #warning "using fork version"
    exportVideoRenderTarget *vrt = new exportVideoRenderTarget(m_project->renderTask());    
#endif
#else
	#warning "should not use this"
    VideoRenderTarget_sV *vrt = new VideoRenderTarget_sV(m_project->renderTask());
#endif
    vrt->setTargetFile(QString(filename));
    vrt->setVcodec(QString(codec));
    m_project->renderTask()->setRenderTarget(vrt); 
    m_renderTargetSet = true;
}

void SlowmoRenderer_sV::setImagesRenderTarget(QString filenamePattern, QString directory)
{
    ImagesRenderTarget_sV *irt = new ImagesRenderTarget_sV(m_project->renderTask());
    irt->setFilenamePattern(QString(filenamePattern));
    irt->setTargetDir(QString(directory));
    m_project->renderTask()->setRenderTarget(irt);
    m_renderTargetSet = true;
}

void SlowmoRenderer_sV::setInterpolation(InterpolationType interpolation)
{
    m_project->renderTask()->renderPreferences().interpolation = interpolation;
}

void SlowmoRenderer_sV::setMotionblur(MotionblurType motionblur)
{
    m_project->renderTask()->renderPreferences().motionblur = motionblur;
}

void SlowmoRenderer_sV::setSize(bool original)
{
    if (original) {
        m_project->renderTask()->renderPreferences().size = FrameSize_Orig;
    } else {
        m_project->renderTask()->renderPreferences().size = FrameSize_Small;
    }
}

void SlowmoRenderer_sV::setV3dLambda(float lambda)
{
    m_project->preferences()->flowV3DLambda() = lambda;
}

void SlowmoRenderer_sV::start()
{
    m_project->renderTask()->slotContinueRendering();
}
void SlowmoRenderer_sV::abort()
{
    m_project->renderTask()->slotStopRendering();
}


void SlowmoRenderer_sV::slotProgressInfo(int progress)
{
    m_lastProgress = progress;
}
void SlowmoRenderer_sV::slotTaskSize(QString desc, int size)
{
    std::cout << desc.toStdString() << std::endl;
    m_taskSize = size;
}

void SlowmoRenderer_sV::slotFinished(QString time)
{
    std::cout << std::endl << "Rendering finished.  Time taken: " << time.toStdString() << std::endl;
}


void SlowmoRenderer_sV::printProgress()
{
    std::cout << m_lastProgress << "/" << m_taskSize << std::endl;
}

bool SlowmoRenderer_sV::isComplete(QString &message) const
{
    bool b = true;
    if (!m_renderTargetSet) {
        b = false;
        message.append("No render target set.\n");
    }
    return b;
}

void SlowmoRenderer_sV::slotNewFrameSourceTask(const QString taskDescription, int taskSize)
{
     std::cout << "slotNewFrameSourceTask";
}

void SlowmoRenderer_sV::slotFrameSourceTasksFinished()
{
        std::cout << "slotFrameSourceTasksFinished";
}


