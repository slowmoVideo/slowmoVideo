/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "videoFrameSource_sV.h"
#include "project_sV.h"

#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QTime>
#include <QtCore/QRegExp>

QRegExp VideoFrameSource_sV::regexFrameNumber("frame=\\s*(\\d+)");

//not sure about that here, means unlimited !
const int tmout = (-1);

/// \todo Check QProcess::exitCode() to find out if ffmpeg worked or not
VideoFrameSource_sV::VideoFrameSource_sV(const Project_sV *project, const QString &filename)
throw(FrameSourceError) :
    AbstractFrameSource_sV(project),
    m_inFile(filename),
    m_fps(1,1),
    m_ffmpegSemaphore(1),
    m_initialized(false),
    cur_frame(0)
{
    if (!QFileInfo(filename).exists()) {
        throw FrameSourceError(tr("Video file %1 does not exist!").arg(filename));
    }

    m_videoInfo = new VideoInfoSV();
    // use copy constructor 
    *m_videoInfo = getInfo(filename.toStdString().c_str());

    if (m_videoInfo->streamsCount <= 0) {
        qDebug() << "Video info is invalid: " << filename;
        throw FrameSourceError(tr("Video is invalid, no streams found in %1").arg(filename));
    }
    m_fps = Fps_sV(m_videoInfo->frameRateNum, m_videoInfo->frameRateDen);



    createDirectories();
    locateFFmpeg();

    m_ffmpeg = new QProcess(this);
    m_timer = new QTimer(this);

    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressUpdate()));
}
VideoFrameSource_sV::~VideoFrameSource_sV()
{
    delete m_ffmpeg;
    delete m_timer;
    delete m_videoInfo;
}


void VideoFrameSource_sV::slotUpdateProjectDir()
{
    //TODO: is it really needed ?
    // Delete old directories if they are empty
    //m_dirFramesSmall.rmdir(".");
    //m_dirFramesOrig.rmdir(".");
    createDirectories();
}

void VideoFrameSource_sV::createDirectories()
{
    m_dirFramesSmall = project()->getDirectory("frames/small");
    m_dirFramesOrig = project()->getDirectory("frames/orig");
}

void VideoFrameSource_sV::initialize()
{
    if (!initialized()) {
        // Start the frame extraction process
        slotExtractSmallFrames();
    }
}
bool VideoFrameSource_sV::initialized() const
{
    return m_initialized;
}

int64_t VideoFrameSource_sV::framesCount() const
{
    return m_videoInfo->framesCount;
}

void VideoFrameSource_sV::setFramesCount(int64_t framesCount) {
    //qDebug() << "setting frameCount "<< framesCount;
    m_videoInfo->framesCount = framesCount;
}

const Fps_sV* VideoFrameSource_sV::fps() const
{
    return &m_fps;
}
QImage VideoFrameSource_sV::frameAt(const uint frame, const FrameSize frameSize)
{
    // TODO:
    QString path = framePath(frame, frameSize);
    //qDebug() << "frameAt "<< frame;
#if 0
     if(frameCache.contains(path))
	     return *(frameCache.object(path));
#endif
    QImage _frame = QImage(path);
#if 0
     frameCache.insert(path, &_frame);
#endif
    return _frame;
}
const QString VideoFrameSource_sV::videoFile() const
{
    return m_inFile.fileName();
}

const QString VideoFrameSource_sV::framePath(const uint frame, const FrameSize frameSize) const
{
    QString dir;
    switch (frameSize) {
    case FrameSize_Orig:
        dir = m_dirFramesOrig.absolutePath();
        break;
    case FrameSize_Small:
    default:
        dir = m_dirFramesSmall.absolutePath();
        break;
    }

    // ffmpeg numbering starts with 1, therefore add 1 to the frame number
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    return QString("%1/frame%2.png").arg(dir).arg(frame+1, 5, 10, QChar::fromAscii('0'));
#else
    return QString("%1/frame%2.png").arg(dir).arg(frame+1, 5, 10, QChar::fromLatin1('0'));
#endif
}

void VideoFrameSource_sV::extractFramesFor(const FrameSize frameSize, QProcess *process)
{
    QStringList args;
    args << "-i" << m_inFile.fileName();
    args << "-f" << "image2";

    if (frameSize == FrameSize_Small) {
        int w = m_videoInfo->width;
        int h = m_videoInfo->height;
        while (w > 600) {
            w /= 2;
            h /= 2;
        }
        qDebug() << "Thumbnail frame size: " << w << "x" << h;
        args << "-s" << QString("%1x%2").arg(w).arg(h);
        args << m_dirFramesSmall.absoluteFilePath("frame%05d.png");
    } else {
        args << m_dirFramesOrig.absoluteFilePath("frame%05d.png");
    }

    qDebug() << "Extracting frames with " << m_settings.value("binaries/ffmpeg", "ffmpeg").toString() << args;
//    {
//        QStringList a2;
//        a2 << "-version";
//        process->start(m_settings.value("binaries/ffmpeg").toString());//, "ffmpeg").toString(), a2);
//        qDebug() << process->readAllStandardOutput();
//        qDebug() << process->readAllStandardError();
//        process->terminate();
//    }
    process->start(m_settings.value("binaries/ffmpeg", "ffmpeg").toString(), args);
    qDebug() << process->readAllStandardOutput();
    qDebug() << process->readAllStandardError();
}

bool VideoFrameSource_sV::rebuildRequired(const FrameSize frameSize)
{
    bool needsRebuild = false;

    QImage frame = frameAt(0, frameSize);
    needsRebuild |= frame.isNull();

    //qDebug() << "last frame to check " << m_videoInfo->framesCount-1;
    // rewind a little bit to account rounding error...
    frame = frameAt(m_videoInfo->framesCount-10, frameSize);
    needsRebuild |= frame.isNull();

    return needsRebuild;
}

void VideoFrameSource_sV::locateFFmpeg()
{
    if (m_avconvInfo.locate(m_settings.value("binaries/ffmpeg", "").toString())) {
        m_settings.setValue("binaries/ffmpeg", m_avconvInfo.executablePath());
        m_settings.sync();
    } else {
        throw FrameSourceError(tr("ffmpeg/avconv executable not found! Cannot load video."
                                  "\n(It is also possible that it took a little long to respond "
                                  "due to high workload, so you might want to try again.)"
                          #ifdef WINDOWS
                                  "\nPlease download the static ffmpeg build from ffmpeg.zeranoe.com "
                                  "and extract ffmpeg.exe in the same directory as slowmoUI.exe."
                          #endif
                                  ));
    }
}

void VideoFrameSource_sV::slotExtractSmallFrames()
{
    emit signalNextTask(tr("Extracting thumbnail-sized frames from the video file"), m_videoInfo->framesCount);
    m_timer->start(100);
    if (rebuildRequired(FrameSize_Small)) {

        m_ffmpegSemaphore.acquire();

        m_ffmpeg->waitForFinished(tmout);
        m_ffmpeg->terminate();

        disconnect(m_ffmpeg, SIGNAL(finished(int)), this, 0);
        connect(m_ffmpeg, SIGNAL(finished(int)), this, SLOT(slotExtractOrigFrames()));

        extractFramesFor(FrameSize_Small, m_ffmpeg);

        m_ffmpegSemaphore.release();

    } else {
        slotExtractOrigFrames();
    }
}

void VideoFrameSource_sV::slotExtractOrigFrames()
{
    emit signalNextTask(tr("Extracting original-sized frames from the video file"), m_videoInfo->framesCount);
    m_timer->start(100);
    if (rebuildRequired(FrameSize_Orig)) {

        m_ffmpegSemaphore.acquire();

        m_ffmpeg->waitForFinished(tmout);
        m_ffmpeg->terminate();

        disconnect(m_ffmpeg, SIGNAL(finished(int)), this, 0);
        connect(m_ffmpeg, SIGNAL(finished(int)), this, SLOT(slotInitializationFinished()));

        extractFramesFor(FrameSize_Orig, m_ffmpeg);

        m_ffmpegSemaphore.release();

    } else {
        slotInitializationFinished();
    }
}

void VideoFrameSource_sV::slotInitializationFinished()
{
    m_timer->stop();
    emit signalAllTasksFinished();

    m_ffmpegSemaphore.acquire();
    m_ffmpeg->waitForFinished(tmout);

    QRegExp regex(regexFrameNumber);
    QString s;
    s = QString(m_ffmpeg->readAllStandardError());
    qDebug() << "slotExtractOrigFrames : " << s << "end";
    if (regex.lastIndexIn(s) >= 0) {
        //fprintf(stderr,"last frame is : %d\n",regex.cap(1).toInt());
        setFramesCount(regex.cap(1).toLong());
    }  else {
        //fprintf(stderr, "last frame not found !\n" );
        qDebug() << "last frame not found";
    }

    m_ffmpeg->terminate();
    m_ffmpegSemaphore.release();

    if (!rebuildRequired(FrameSize_Small) && !rebuildRequired(FrameSize_Orig)) {
        m_initialized = true;
    }
}

void VideoFrameSource_sV::slotAbortInitialization()
{
    m_ffmpegSemaphore.acquire();
    if (m_ffmpeg != NULL) {
        m_ffmpeg->terminate();
    }
    m_ffmpegSemaphore.release();

}

void VideoFrameSource_sV::slotProgressUpdate()
{
    QRegExp regex(regexFrameNumber);
    QString s;
    m_ffmpegSemaphore.acquire();
    s = QString(m_ffmpeg->readAllStandardError());
    if (regex.lastIndexIn(s) >= 0) {
        emit signalTaskProgress(regex.cap(1).toInt());
        emit signalTaskItemDescription(tr("Frame %1 of %2").arg(regex.cap(1)).arg(m_videoInfo->framesCount));
    }
    m_ffmpegSemaphore.release();
}


void VideoFrameSource_sV::loadOrigFrames()
{
    
    m_ffmpegSemaphore.acquire();
    
    m_ffmpeg->waitForFinished(tmout);
    m_ffmpeg->terminate();
    
	QTime time;
        time.start();
    
    extractFramesFor(FrameSize_Orig, m_ffmpeg);
    
    m_ffmpeg->waitForFinished(tmout);
    m_ffmpeg->terminate();
   
    qDebug() << "ffmpeg in  " << time.elapsed()  << "ms";

    m_ffmpegSemaphore.release();
    
    
}
