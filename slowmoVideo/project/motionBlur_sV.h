#ifndef MOTIONBLUR_SV_H
#define MOTIONBLUR_SV_H

#include <QtCore/QDir>
#include <QtGui/QImage>
#include "../lib/defs_sV.hpp"
class Project_sV;

class MotionBlur_sV
{
public:
    MotionBlur_sV(Project_sV *project);

    QImage blur(float startFrame, float endFrame, float shutterPercent, FrameSize size);

public slots:
    void slotUpdateProjectDir();

private:
    Project_sV *m_project;
    QDir m_dirCacheSmall;
    QDir m_dirCacheOrig;

    QString cachedFramePath(float framePos, FrameSize size);
    void createDirectories();
};

#endif // MOTIONBLUR_SV_H
