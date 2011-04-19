#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include <QFile>
#include <QDir>
#include <QString>
#include <QImage>

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class Project_sV
{
public:
    Project_sV(QString filename, QString projectDir);

    const VideoInfoSV& videoInfo() const;

    /**
      Extracts the frames from the video file into single images
      */
    bool extractFrames() const;
    /**
      Checks the availability of the frames and decides
      whether they need to be extracted with extractFrames()
      */
    bool rebuildRequired() const;
    /**
      @return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    QImage frameAt(uint frame) const;

private:
    static QString defaultFramesDir;

    bool m_canWriteFrames;

    QFile m_inFile;
    QDir m_projDir;
    QDir m_framesDir;
    VideoInfoSV m_videoInfo;
};

#endif // PROJECT_SV_H
