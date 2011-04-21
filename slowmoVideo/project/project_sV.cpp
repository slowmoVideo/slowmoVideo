#include "project_sV.h"

#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFileInfo>

QString Project_sV::defaultFramesDir("frames");
QString Project_sV::defaultThumbFramesDir("framesThumb");

Project_sV::Project_sV(QString filename, QString projectDir) :
    QObject(),
    m_canWriteFrames(false),
    m_inFile(filename),
    m_projDir(projectDir),
    m_ffmpegOrig(NULL),
    m_ffmpegSmall(NULL)
{
    m_videoInfo = getInfo(filename.toStdString().c_str());
    if (m_videoInfo.streamsCount <= 0) {
        qDebug() << "Video info is invalid.";
    }

    // Create directories if necessary
    qDebug() << "Project directory: " << m_projDir.absolutePath();
    if (!m_projDir.exists()) {
        m_projDir.mkpath(".");
    }
    if (!m_projDir.exists(defaultFramesDir)) {
        m_projDir.mkdir(defaultFramesDir);
    }
    m_framesDir = QDir(m_projDir.absolutePath() + "/" + defaultFramesDir);
    if (!m_projDir.exists(defaultThumbFramesDir)) {
        m_projDir.mkdir(defaultThumbFramesDir);
    }
    m_thumbFramesDir = QDir(m_projDir.absolutePath() + "/" + defaultThumbFramesDir);

    m_canWriteFrames = validDirectories();


    m_timer = new QTimer(this);
    bool b = true;
    b &= connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressUpdate()));
    Q_ASSERT(b);
}

bool Project_sV::validDirectories() const
{
    bool valid = true;
    if (!m_projDir.exists()) {
        qDebug() << "Project directory could not be created.";
        valid = false;
    }
    if (!m_framesDir.exists()) {
        qDebug() << "Could not change to directory " << m_projDir.absolutePath() << "/" << defaultFramesDir;
        valid = false;
    } else {
        qDebug() << "Frame directory: " << m_framesDir.absolutePath();
    }
    if (!m_thumbFramesDir.exists()) {
        qDebug() << "Could not change to directory " << m_projDir.absolutePath() << "/" << defaultThumbFramesDir;
        valid = false;
    } else {
        qDebug() << "Frame thumb directory: " << m_thumbFramesDir.absolutePath();
    }
    return valid;
}

const VideoInfoSV& Project_sV::videoInfo() const
{
    return m_videoInfo;
}

void Project_sV::extractFrames(bool force)
{
    m_timer->start(100);
    if (rebuildRequired(FrameSize_Orig) || force) {
        extractFramesFor(FrameSize_Orig);
    } else {
        emit signalFramesExtracted(Project_sV::FrameSize_Orig);
    }
    if (rebuildRequired(FrameSize_Small) || force) {
        extractFramesFor(FrameSize_Small);
    } else {
        emit signalFramesExtracted(Project_sV::FrameSize_Small);
    }
}

bool Project_sV::extractFramesFor(const FrameSize frameSize)
{
    bool success = false;
    if (m_canWriteFrames) {
        QProcess *ffmpeg = new QProcess();

        QStringList args;
        args << "-i" << m_inFile.fileName();
        args << "-f" << "image2";
        args << "-sameq";

        switch (frameSize) {
        case FrameSize_Orig:
            args << QString("%1/frame%5d.jpg").arg(m_framesDir.absolutePath());
            if (m_ffmpegOrig != NULL && m_ffmpegOrig->state() != QProcess::NotRunning) {
                qDebug() << "Shutting down old ffmpeg process";
                m_ffmpegOrig->waitForFinished(2000);
            }
            m_ffmpegOrig = ffmpeg;
            break;
        case FrameSize_Small:
            {
                int w = m_videoInfo.width;
                int h = m_videoInfo.height;
                while (w > 640) {
                    w /= 2;
                    h /= 2;
                }
                qDebug() << "Thumbnail frame size: " << w << "x" << h;
                args << "-s" << QString("%1x%2").arg(w).arg(h);
                args << QString("%1/frame%5d.jpg").arg(m_thumbFramesDir.absolutePath());
                if (m_ffmpegSmall != NULL && m_ffmpegSmall->state() != QProcess::NotRunning) {
                    qDebug() << "Shutting down old ffmpeg process";
                    m_ffmpegSmall->waitForFinished(2000);
                }
                m_ffmpegSmall = ffmpeg;
            }
            break;
        }
        bool b = true;
        b &= connect(ffmpeg, SIGNAL(finished(int)), this, SLOT(slotExtractingFinished()));
        Q_ASSERT(b);


        ffmpeg->start("ffmpeg", args);
        qDebug() << ffmpeg->readAllStandardOutput();
        qDebug() << ffmpeg->readAllStandardError();

        if (ffmpeg->exitCode() == 0) {
            success = true;
        }

    }
    return success;
}

bool Project_sV::rebuildRequired(const FrameSize frameSize) const
{
    bool needsRebuild = false;

    QImage frame = frameAt(1, frameSize); // Counting starts with 1!
    needsRebuild |= frame.isNull();

    frame = frameAt(m_videoInfo.framesCount, frameSize);
    needsRebuild |= frame.isNull();

    return needsRebuild;
}

QImage Project_sV::frameAt(const uint frame, const FrameSize frameSize) const
{
    QString filename;
    switch (frameSize) {
    case FrameSize_Orig:
        filename = QString(m_framesDir.absoluteFilePath("frame%1.jpg").arg(frame, 5, 10, QChar::fromAscii('0')));
        break;
    case FrameSize_Small:
        filename = QString(m_thumbFramesDir.absoluteFilePath("frame%1.jpg").arg(frame, 5, 10, QChar::fromAscii('0')));
        break;
    }

    return QImage(filename);
}

void Project_sV::slotExtractingFinished()
{
    bool allFinished = true;
    qDebug() << "Extracting finished!";
    if (m_ffmpegOrig->state() == QProcess::NotRunning) {
        qDebug() << "Orig size thread not running.";
        emit signalFramesExtracted(Project_sV::FrameSize_Orig);
    } else { allFinished = false; }
    if (m_ffmpegSmall->state() == QProcess::NotRunning) {
        qDebug() << "Thumbnail size thread not running.";
        emit signalFramesExtracted(Project_sV::FrameSize_Small);
    } else { allFinished = false; }

    if (!allFinished && m_ffmpegOrig != NULL && m_ffmpegSmall != NULL) {
        qDebug() << "ffmpeg Orig: " << m_ffmpegOrig->readAllStandardError();
        qDebug() << "ffmpeg Small: " << m_ffmpegSmall->readAllStandardError();
    } else {
        qDebug() << "All finished, stopping timer.";
        m_timer->stop();
    }
}

void Project_sV::slotProgressUpdate()
{
    qDebug() << "=====Timer=====";
    if (m_ffmpegOrig != NULL) {
        qDebug() << "ffmpeg Orig: " << m_ffmpegOrig->readAllStandardError();
    }
    if (m_ffmpegSmall != NULL) {
        qDebug() << "ffmpeg Small: " << m_ffmpegSmall->readAllStandardError();
    }
    qDebug() << "====/Timer=====";
}

QDebug operator<<(QDebug qd, const Project_sV::FrameSize &frameSize)
{
    switch(frameSize) {
    case Project_sV::FrameSize_Orig:
        qd << "Original frame size";
        break;
    case Project_sV::FrameSize_Small:
        qd << "Small frame size";
        break;
    }
    return qd;
}
