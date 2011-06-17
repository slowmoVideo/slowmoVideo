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
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class QProcess;
class Project_sV;

class NoVideoStreamsException {};

/** \brief Uses frames from a video file */
class VideoFrameSource_sV : public AbstractFrameSource_sV
{
    Q_OBJECT
public:
    VideoFrameSource_sV(const Project_sV *project, const QString &filename) throw(NoVideoStreamsException);
    ~VideoFrameSource_sV();

    virtual void initialize();

    int64_t framesCount() const;
    int frameRateNum() const;
    int frameRateDen() const;
    QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig);

private:
    static QRegExp regexFrameNumber;

private:
    QFile m_inFile;
    QDir m_dirFramesSmall;
    QDir m_dirFramesOrig;

    VideoInfoSV *m_videoInfo;

    QTimer *m_timer;
    QProcess *m_ffmpeg;


    const QString framesDirStr(FrameSize frameSize) const;
    /**
      Extracts the frames from the video file into single images
      */
    void extractFramesFor(const FrameSize frameSize, QProcess *process);
    /**
      Checks the availability of the frames and decides
      whether they need to be extracted with extractFrames()
      */
    bool rebuildRequired(const FrameSize frameSize);

    const QString frameFileStr(int number, FrameSize size) const;

signals:
    void signalExtractOrigFramesFinished();
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
