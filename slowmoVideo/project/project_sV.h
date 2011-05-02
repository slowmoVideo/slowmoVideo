#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include <QFile>
#include <QDir>
#include <QString>
#include <QImage>

#include <QObject>
#include <QFile>

#include "../lib/opticalFlowBuilder_sV.h"
#include "../lib/defs_sV.h"

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class QSignalMapper;
class QProcess;
class QRegExp;
class QTimer;
class Project_sV : public QObject
{
    Q_OBJECT

public:
    Project_sV(QString filename, QString projectDir);
    ~Project_sV();

    const VideoInfoSV& videoInfo() const;

    enum FrameSize { FrameSize_Orig, FrameSize_Small };

    /**
      @return true, iff all required directories exist
      */
    bool validDirectories() const;
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
    /**
      @return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) const;

    QFile* frameFile(int number) const;
    QFile* thumbFile(int number) const;
    QFile* flowFile(int number, FlowDirection direction) const;

    const QString frameFileStr(int number) const;
    const QString thumbFileStr(int number) const;
    const QString flowFileStr(int number, FlowDirection direction) const;

private:
    static QString defaultFramesDir;
    static QString defaultThumbFramesDir;
    static QString defaultFlowDir;
    static QRegExp regexFrameNumber;

    struct {
        bool origFinished;
        bool smallFinished;
        void reset() {
            origFinished = false;
            smallFinished = false;
        }
        bool allFinished() { return origFinished && smallFinished; }
    } m_processStatus;

    bool m_canWriteFrames;

    QFile m_inFile;
    QDir m_projDir;
    QDir m_framesDir;
    QDir m_thumbFramesDir;
    QDir m_flowDir;
    VideoInfoSV m_videoInfo;

    QSignalMapper *m_signalMapper;
    QProcess *m_ffmpegOrig;
    QProcess *m_ffmpegSmall;
    QTimer *m_timer;



private slots:
    void slotExtractingFinished(int);
    /**
      Checks the progress of the ffmpeg threads by reading their stderr
      and emits signalProgressUpdated() if necessary.
      */
    void slotProgressUpdate();

signals:
    /**
      Emitted when all frames have been extracted for this frame size.
      */
    void signalFramesExtracted(Project_sV::FrameSize frameSize);
    /**
      Emitted when an ffmpeg thread has made progress (i.e. wrote to stderr).
      @param progress Number in the range 0...100
      */
    void signalProgressUpdated(Project_sV::FrameSize frameSize, int progress);

};

QDebug operator<<(QDebug qd, const Project_sV::FrameSize &frameSize);

#endif // PROJECT_SV_H
