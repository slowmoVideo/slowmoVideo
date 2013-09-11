#ifndef AVCONVINFO_H
#define AVCONVINFO_H

#include "../lib/defs_sV.hpp"
#include <QtCore/QString>

class AvconvInfo
{
public:
    enum Distribution { Dist_ffmpeg, Dist_avconv };


    static bool testAvconvExecutable(QString path);


    AvconvInfo();

    bool locate(QString executablePath = "");

    QString executablePath() const;
    QString optionSameQuant() const;

    Distribution distribution() const;
    void printInfo() const;

private:
    void identify();

    QString m_executablePath;
    Distribution m_dist;
};

#endif // AVCONVINFO_H
