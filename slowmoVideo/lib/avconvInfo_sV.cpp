#include "avconvInfo_sV.h"

#include "QProcess"
#include "QFileInfo"

AvconvInfo::AvconvInfo() :
    m_executablePath(""),
    m_dist(Dist_avconv)
{}


bool AvconvInfo::testAvconvExecutable(QString path)
{
    QProcess ffmpeg(NULL);
    QStringList args;
    args << "-version";
    ffmpeg.start(path, args);
    ffmpeg.waitForFinished(5000);
    QByteArray output = ffmpeg.readAllStandardOutput();
    return output.size() > 0;

}
bool AvconvInfo::locate(QString executablePath)
{
    QStringList paths;

    if (executablePath.size() > 0) {
        paths << executablePath;
    }

    paths
          << "avconv"
          << "ffmpeg"
         #ifndef WINDOWS
          << "/usr/bin/avconv"
          << "/usr/bin/ffmpeg"
          << "/usr/local/bin/avconv"
          << "/usr/local/bin/ffmpeg"
         #endif
             ;

    bool found = false;
    foreach (QString path, paths) {
        if (testAvconvExecutable(path)) {
            m_executablePath = path;
            found = true;
            identify();
            break;
        }
    }

    if (found) {
        printInfo();
    } else {
        qDebug() << "Did not find avconv/ffmpeg. Searched at the following locations:";
        foreach (QString path, paths) {
            qDebug() << "* " << QFileInfo(path).absoluteFilePath();
        }
    }

    return found;
}

void AvconvInfo::identify()
{
    QProcess ffmpeg(NULL);
    QStringList args;
    args << "-version";
    ffmpeg.start(m_executablePath, args);
    ffmpeg.waitForFinished(5000);
    QByteArray output = ffmpeg.readAllStandardOutput();
    if (output.indexOf("avconv") >= 0) {
        m_dist = Dist_avconv;
    } else {
        m_dist = Dist_ffmpeg;
    }
}

void AvconvInfo::printInfo() const
{
    qDebug() << "Found avconv/ffmpeg executable at " << QFileInfo(m_executablePath).absoluteFilePath() <<
                "\n\tDistribution: " << (m_dist == Dist_avconv ? "avconv" : "ffmpeg");
}

QString AvconvInfo::executablePath() const
{
    return m_executablePath;
}
QString AvconvInfo::optionSameQuant() const
{
    if (m_dist == Dist_avconv) {
        return "-same_quant";
    } else {
        return "-sameq";
    }
}
