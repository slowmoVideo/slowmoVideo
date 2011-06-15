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

class VideoFrameSource_sV : public AbstractFrameSource_sV
{
public:
    VideoFrameSource_sV(const Project_sV *project, const QString &filename) throw(NoVideoStreamsException);
    ~VideoFrameSource_sV();

    int64_t framesCount() const;
    int frameRateNum() const;
    int frameRateDen() const;
    QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) const;

signals:
    /**
      Emitted when all frames have been extracted for this frame size.
      */
    void signalFramesExtracted(FrameSize frameSize);
    /**
      Emitted when an ffmpeg thread has made progress (i.e. wrote to stderr).
      @param progress Number in the range 0...100
      */
    void signalProgressUpdated(FrameSize frameSize, int progress);

private:
    QFile m_inFile;
    QDir m_dirFramesSmall;
    QDir m_dirFramesOrig;

    VideoInfoSV *m_videoInfo;
    QTimer *m_timer;

    // TODO initialize, delete ...
    QProcess *m_ffmpegOrig;
    QProcess *m_ffmpegSmall;


    struct {
        bool origFinished;
        bool smallFinished;
        void reset() {
            origFinished = false;
            smallFinished = false;
        }
        bool allFinished() { return origFinished && smallFinished; }
    } m_processStatus;


    const QString framesDirStr(FrameSize frameSize) const;
    /**
      Extracts frames for all sizes.
      @param force Also generate frames if they already exist.
      */
    void extractFrames(bool force = false);
    /**
      Extracts the frames from the video file into single images
      */
    bool extractFramesFor(const FrameSize frameSize);
    /**
      Checks the availability of the frames and decides
      whether they need to be extracted with extractFrames()
      */
    bool rebuildRequired(const FrameSize frameSize) const;

};

#endif // VIDEOFRAMESOURCE_SV_H
