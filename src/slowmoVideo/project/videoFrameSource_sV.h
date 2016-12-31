/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef VIDEOFRAMESOURCE_SV_H
#define VIDEOFRAMESOURCE_SV_H

#include "abstractFrameSource_sV.h"
#include "../lib/defs_sV.hpp"
#include "../lib/avconvInfo_sV.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <QtCore/QSemaphore>

#include "../lib/videoInfo_sV.h"

class QProcess;
class Project_sV;

/**
  \brief Uses frames from a video file
  \todo Use libav directly for frame extraction? (not the ffmpeg command)
  \todo Extract full frames only before rendering, only used ones
 */
class VideoFrameSource_sV : public AbstractFrameSource_sV
{
    Q_OBJECT
public:
    /** Builds a new video frame source from the given file. */
    VideoFrameSource_sV(const Project_sV *project, const QString &filename)
    throw(FrameSourceError);

    ~VideoFrameSource_sV();

    void initialize();
    bool initialized() const;

    int64_t framesCount() const;
    void setFramesCount(int64_t framesCount);

    const Fps_sV* fps() const;
    QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig);
    const QString framePath(const uint frame, const FrameSize frameSize) const;

    /** \return The absolute path of the input video file. */
    const QString videoFile() const;

    void loadOrigFrames();
    
public slots:
    void slotAbortInitialization();
    void slotUpdateProjectDir();

private:
    static QRegExp regexFrameNumber;

private:
    QFile m_inFile;
    QDir m_dirFramesSmall;
    QDir m_dirFramesOrig;
    QSettings m_settings;
    AvconvInfo m_avconvInfo;

    VideoInfoSV *m_videoInfo;
    Fps_sV m_fps;

    QTimer *m_timer;
    QProcess *m_ffmpeg;
    QSemaphore m_ffmpegSemaphore;
    bool m_initialized;
    int64_t cur_frame;

    void createDirectories();
    /**
      Extracts the frames from the video file into single images
      */
    void extractFramesFor(const FrameSize frameSize, QProcess *process);
    /**
      Checks the availability of the frames and decides
      whether they need to be extracted with extractFrames()
      */
    bool rebuildRequired(const FrameSize frameSize);

    void locateFFmpeg();

public:
    static bool testFfmpegExecutable(QString path);

signals:
    /** Emitted when the task for extracting original-sized images has finished (or has been terminated) */
    void signalExtractOrigFramesFinished();
    /** Emitted when the task for extracting thumbnail-sized images has finished (or has been terminated) */
    void signalExtractSmallFramesFinished();

private slots:
    void slotExtractOrigFrames();
    void slotExtractSmallFrames();
    void slotInitializationFinished();
    /**
      Checks the progress of the ffmpeg threads by reading their stderr
      and emits signalTaskProgress() and signalTaskItemDescription() if necessary.
      */
    void slotProgressUpdate();

};

#endif // VIDEOFRAMESOURCE_SV_H
