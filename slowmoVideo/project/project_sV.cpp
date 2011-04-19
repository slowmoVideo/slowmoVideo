#include "project_sV.h"

#include <QProcess>
#include <QDebug>
#include <QFileInfo>

QString Project_sV::defaultFramesDir("frames");

Project_sV::Project_sV(QString filename, QString projectDir) :
    m_canWriteFrames(false),
    m_inFile(filename),
    m_projDir(projectDir)
{
    m_videoInfo = getInfo(filename.toStdString().c_str());
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (m_videoInfo.streamsCount <= 0) {
        qDebug() << "Video info is invalid.";
    }

    if (!m_projDir.exists(defaultFramesDir)) {
        m_projDir.mkdir(defaultFramesDir);
    }
    m_framesDir = QDir(m_projDir);
    if (!m_framesDir.cd(defaultFramesDir)) {
        qDebug() << "Could not change to directory " << m_projDir.absolutePath() << "/" << defaultFramesDir;
    } else {
        m_canWriteFrames = true;
        qDebug() << "Frame directory: " << m_framesDir.absolutePath();
    }

    if (rebuildRequired()) {
        qDebug() << "Re-build required.";
        extractFrames();
    }
}

const VideoInfoSV& Project_sV::videoInfo() const
{
    return m_videoInfo;
}

bool Project_sV::extractFrames() const
{
    bool success = false;
    if (m_canWriteFrames) {
        QProcess ffmpeg;

        ffmpeg.start("ffmpeg", QStringList() << "-i" << m_inFile.fileName() << "-f" << "image2" << "-sameq"
                     << QString("%1/frame%5d.jpg").arg(m_framesDir.absolutePath()));
        ffmpeg.waitForFinished();
        qDebug() << ffmpeg.readAllStandardOutput();
        qDebug() << ffmpeg.readAllStandardError();

        if (ffmpeg.exitCode() == 0) {
            success = true;
        }

    }
    return success;
}

bool Project_sV::rebuildRequired() const
{
    bool needsRebuild = false;

    QImage frame = frameAt(1); // Counting starts with 1!
    needsRebuild |= frame.isNull();

    frame = frameAt(m_videoInfo.framesCount);
    needsRebuild |= frame.isNull();

    return needsRebuild;
}

QImage Project_sV::frameAt(uint frame) const
{
    QString filename(m_framesDir.absoluteFilePath("frame%1.jpg").arg(frame, 5, 10, QChar::fromAscii('0')));
    return QImage(filename);
}
