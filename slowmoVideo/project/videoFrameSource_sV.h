#ifndef VIDEOFRAMESOURCE_SV_H
#define VIDEOFRAMESOURCE_SV_H

#include "abstractFrameSource_sV.h"
#include <QtCore/QFile>

class VideoFrameSource_sV : public AbstractFrameSource_sV
{
public:
    VideoFrameSource_sV(const QDir &projectDir);
    void loadFile(QString filename);

private:
    QFile m_inFile;
};

#endif // VIDEOFRAMESOURCE_SV_H
