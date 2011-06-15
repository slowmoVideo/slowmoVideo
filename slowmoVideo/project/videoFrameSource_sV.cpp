#include "videoFrameSource_sV.h"
#include "project_sV.h"


#include <QtCore/QProcess>

VideoFrameSource_sV::VideoFrameSource_sV(const Project_sV *project, const QString &filename) throw(NoVideoStreamsException) :
    AbstractFrameSource_sV(project),
    m_inFile(filename)
{
    m_videoInfo = new VideoInfoSV();

    *m_videoInfo = getInfo(filename.toStdString().c_str());
    if (m_videoInfo->streamsCount <= 0) {
        qDebug() << "Video info is invalid: " << filename;
        throw NoVideoStreamsException();
    }

    m_dirFramesSmall = project->getDirectory("framesSmall");
    m_dirFramesOrig = project->getDirectory("framesOrig");
}
VideoFrameSource_sV::~VideoFrameSource_sV()
{
    // TODO
}

int64_t VideoFrameSource_sV::framesCount() const
{
    return m_videoInfo->framesCount;
}
int VideoFrameSource_sV::frameRateNum() const
{
    return m_videoInfo->frameRateNum;
}
int VideoFrameSource_sV::frameRateDen() const
{
    return m_videoInfo->frameRateDen;
}
QImage VideoFrameSource_sV::frameAt(const uint frame, const FrameSize frameSize) const
{
    // TODO
    return QImage();
}


void VideoFrameSource_sV::extractFrames(bool force)
{
//    m_timer->start(100);
//    m_processStatus.reset();
//    if (rebuildRequired(FrameSize_Orig) || force) {
//        extractFramesFor(FrameSize_Orig);
//    } else {
//        m_processStatus.origFinished = true;
//        emit signalFramesExtracted(FrameSize_Orig);
//    }
//    if (rebuildRequired(FrameSize_Small) || force) {
//        extractFramesFor(FrameSize_Small);
//    } else {
//        m_processStatus.smallFinished = true;
//        emit signalFramesExtracted(FrameSize_Small);
//    }
//    if (m_processStatus.allFinished()) {
//        m_timer->stop();
//    }
}

bool VideoFrameSource_sV::extractFramesFor(const FrameSize frameSize)
{
    bool success = false;
    QProcess *ffmpeg = new QProcess();

    QStringList args;
    args << "-i" << m_inFile.fileName();
    args << "-f" << "image2";
    args << "-sameq";

//    switch (frameSize) {
//    case FrameSize_Orig:
//        {
//            args << QString("%1/frame%5d.jpg").arg(framesDirStr(frameSize));
//            if (m_ffmpegOrig != NULL && m_ffmpegOrig->state() != QProcess::NotRunning) {
//                qDebug() << "Shutting down old ffmpeg process";
//                m_ffmpegOrig->waitForFinished(2000);
//                // TODO signals
////                m_signalMapper->disconnect(m_ffmpegOrig, 0,0,0);
//                delete m_ffmpegOrig;
//                m_ffmpegOrig = NULL;
//            }
//            m_ffmpegOrig = ffmpeg;

////            m_signalMapper->setMapping(m_ffmpegOrig, (int)FrameSize_Orig);
//            bool b = true;
////            b &= connect(m_ffmpegOrig, SIGNAL(finished(int)), m_signalMapper, SLOT(map()));
//            Q_ASSERT(b);
//        }
//        break;
//    case FrameSize_Small:
//        {
//            int w = m_videoInfo->width;
//            int h = m_videoInfo->height;
//            // @todo adjust size
//            while (w > 320) {
//                w /= 2;
//                h /= 2;
//            }
//            qDebug() << "Thumbnail frame size: " << w << "x" << h;
//            args << "-s" << QString("%1x%2").arg(w).arg(h);
//            args << QString("%1/frame%5d.png").arg(framesDirStr(frameSize));
//            if (m_ffmpegSmall != NULL && m_ffmpegSmall->state() != QProcess::NotRunning) {
//                qDebug() << "Shutting down old ffmpeg process";
//                m_ffmpegSmall->waitForFinished(2000);
////                m_signalMapper->disconnect(m_ffmpegSmall, 0,0,0);
//                delete m_ffmpegSmall;
//                m_ffmpegSmall = NULL;
//            }
//            m_ffmpegSmall = ffmpeg;

////            m_signalMapper->setMapping(m_ffmpegSmall, (int)FrameSize_Small);
//            bool b = true;
////            b &= connect(m_ffmpegSmall, SIGNAL(finished(int)), m_signalMapper, SLOT(map()));
//            Q_ASSERT(b);

//        }
//        break;
//    }


    ffmpeg->start("ffmpeg", args);
    qDebug() << ffmpeg->readAllStandardOutput();
    qDebug() << ffmpeg->readAllStandardError();

    if (ffmpeg->exitCode() == 0) {
        success = true;
    }

    return success;
}
