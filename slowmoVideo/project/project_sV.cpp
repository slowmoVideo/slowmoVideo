#include "project_sV.h"

Project_sV::Project_sV(const char filename[]) :
    m_inFile(filename)
{
    m_videoInfo = getInfo(filename);
}

const VideoInfoSV& Project_sV::videoInfo() const
{
    return m_videoInfo;
}
