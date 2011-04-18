#include "project_sV.h"

#include <QProcess>
#include <QDebug>
#include <QFileInfo>

Project_sV::Project_sV(const char filename[], const char projectDir[]) :
    m_inFile(filename),
    m_projDir(projectDir)
{
    m_videoInfo = getInfo(filename);
    qDebug() << "Project directory: " << m_projDir.absolutePath();
}

const VideoInfoSV& Project_sV::videoInfo() const
{
    return m_videoInfo;
}

bool Project_sV::extractFrames() const
{
    QProcess ffmpeg;
    if (!m_projDir.exists("frames")) {
        m_projDir.mkdir("frames");
    }
    QDir framesDir(m_projDir);
    if (!framesDir.cd("frames")) {
        qDebug() << "Could not change to directory " << m_projDir.absolutePath() << "/frames";
        return false;
    }
    ffmpeg.start("ffmpeg", QStringList() << "-i" << m_inFile.fileName() << "-f" << "image2"
                 << QString("%1/img%4d.jpg").arg(framesDir.absolutePath())
                 << "--sameq");
    ffmpeg.waitForFinished();
    qDebug() << ffmpeg.readAllStandardOutput();
    qDebug() << ffmpeg.readAllStandardError();

    return true;
}
