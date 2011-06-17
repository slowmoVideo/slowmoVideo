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
#include <QtCore/QDir>
#include <inttypes.h>
class Project_sV;

class Div0Exception {};

/** \brief Represents a source for input frames, like a video or an image sequence */
class AbstractFrameSource_sV : public QObject
{
    Q_OBJECT
public:
    AbstractFrameSource_sV(const Project_sV *project);
    virtual ~AbstractFrameSource_sV();

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
    virtual void initialize();
    virtual bool initialized();

    virtual int64_t framesCount() const = 0;
    virtual int frameRateNum() const = 0;
    virtual int frameRateDen() const = 0;
    float fps() const throw(Div0Exception);

    /**
      @return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    virtual QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) = 0;

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

protected:
    const Project_sV *m_project;

private:
    bool m_initialized;

};


#endif // ABSTRACTFRAMESOURCE_SV_H
