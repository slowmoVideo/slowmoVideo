#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include <QFile>
#include <QDir>
#include <QString>

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class Project_sV
{
public:
    Project_sV(const char filename[], const char projectDir[]);

    const VideoInfoSV& videoInfo() const;

    bool extractFrames() const;

private:
    QFile m_inFile;
    QDir m_projDir;
    VideoInfoSV m_videoInfo;
};

#endif // PROJECT_SV_H
