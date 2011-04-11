#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include <QFile>
#include <QString>

extern "C" {
#include "../lib/videoInfo_sV.h"
}

class Project_sV
{
public:
    Project_sV(const char filename[]);

    const VideoInfoSV& videoInfo() const;

private:
    QFile m_inFile;
    VideoInfoSV m_videoInfo;
};

#endif // PROJECT_SV_H
