/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef ABSTRACTFRAMESOURCE_SV_H
#define ABSTRACTFRAMESOURCE_SV_H

#include "../lib/defs_sV.hpp"

#include <QImage>
#include <QCache>
#include <QtCore/QDir>

class Project_sV;

class Div0Exception {};

/** \brief Represents a source for input frames, like a video or an image sequence */
class AbstractFrameSource_sV : public QObject
{
    Q_OBJECT
public:
    AbstractFrameSource_sV(const Project_sV *project);
    virtual ~AbstractFrameSource_sV();

    const Project_sV* project() { return m_project; }

    /**
      \fn initialize()
      Initializes the frame source (like extracting frames from a video).
      When re-implementing this method, the parent function must be called in order for
      initialized() to work.
      \see signalNextTask() and the other signals
      */
    /**
      \fn initialized()
      \return \c true, if the frame source has been initialized.
      */
    virtual void initialize() = 0;
    virtual bool initialized() const = 0;

    /**
      \fn framesCount()
      \return Number of frames in this frame source
      */
    virtual int64_t framesCount() const = 0;
    virtual const Fps_sV* fps() const = 0;
    double maxTime() const throw(Div0Exception);

    /*
     * add a qcache here for perf loading
     */
    QCache<QString,QImage> frameCache;
    // use it in frameAt
    /**
      \fn frameAt()
      \return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    /**
      \fn framePath()
      \return The path to the frame at position \c frame
      */
    virtual QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) = 0;
    virtual const QString framePath(const uint frame, const FrameSize frameSize = FrameSize_Orig) const = 0;

signals:
    /**
      \fn void signalNextTask(const QString taskDescription, int taskSize)
      A new task has been started, like extracting frames from a video when loading a new frame source.
      \param taskSize Number of task items in this task (e.g. number of frames to extract from a video file)
      */
    /**
      \fn void signalTaskProgress(int progress)
      The current task has made progress.
      \param progress Task item that has been completed. Should be smaller than taskSize given in signalNextTask().
      */
    /**
      \fn void signalTaskItemDescription(const QString desc)
      \param desc Description for the current task (like the file name of an extracted frame)
      */
    /**
      \fn void signalAllTasksFinished()
      All due tasks have been completed.
      */
    void signalNextTask(const QString taskDescription, int taskSize);
    void signalTaskProgress(int progress);
    void signalTaskItemDescription(const QString desc);
    void signalAllTasksFinished();

public slots:
    /**
      \fn slotAbortInitialization()
      Should abort the initialization of the frame source since this might take a long time
      (e.g. extracting large frames from a long video, or re-sizing lots of frames).
      */
    /**
      \fn slotUpdateProjectDir()
      Informs the frame source that the project directory has changed. If the frame source created sub-directories
      in the old project directories, it can e.g. delete them and create them at the new place.
      */
    virtual void slotAbortInitialization() = 0;
    virtual void slotUpdateProjectDir() = 0;

protected:
    const Project_sV *m_project;

};


#endif // ABSTRACTFRAMESOURCE_SV_H
