#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include <QFile>
#include <QDir>
#include <QString>
#include <QImage>

#include <QObject>

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class QProcess;
class QTimer;
class Project_sV : public QObject
{
    Q_OBJECT

public:
    Project_sV(QString filename, QString projectDir);

    const VideoInfoSV& videoInfo() const;

    enum FrameSize { FrameSize_Orig, FrameSize_Small };


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

private:
    static QString defaultFramesDir;
    static QString defaultThumbFramesDir;

    bool m_canWriteFrames;

    QFile m_inFile;
    QDir m_projDir;
    QDir m_framesDir;
    QDir m_thumbFramesDir;
    VideoInfoSV m_videoInfo;

    QProcess *m_ffmpegOrig;
    QProcess *m_ffmpegSmall;
    QTimer *m_timer;



private slots:
    void slotExtractingFinished();
    void slotProgressUpdate();

signals:
    void signalFramesExtracted(Project_sV::FrameSize frameSize);
};

QDebug operator<<(QDebug qd, const Project_sV::FrameSize &frameSize);

#endif // PROJECT_SV_H
