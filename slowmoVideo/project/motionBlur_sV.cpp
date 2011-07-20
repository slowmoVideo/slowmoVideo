#include "motionBlur_sV.h"
#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "../lib/shutter_sV.h"

MotionBlur_sV::MotionBlur_sV(Project_sV *project) :
    m_project(project)
{
    createDirectories();
}

QImage MotionBlur_sV::blur(float startFrame, float endFrame, float shutterPercent, FrameSize size)
{
    float low, high;
    if (startFrame < endFrame) {
        low = startFrame; high = endFrame;
    } else {
        low = endFrame; high = startFrame;
    }

    QStringList frameList;

    float pos = ceil(4*low)/4.0;
    pos = qMax(float(0), pos);
    float endPos = low + shutterPercent * (high-low);
    endPos = qMin(endPos, (float)m_project->frameSource()->framesCount());


    qDebug() << "start: " << startFrame << ", end: " << endFrame << ", low: " << low
             << ", high: " << high << ", percent: " << shutterPercent;
    qDebug() << "pos: " << pos << ", endPos: " << endPos;
    while (pos < endPos) {
        qDebug() << "pos: " << pos;
        frameList << cachedFramePath(pos, size);
        pos += .25;
    }

    return Shutter_sV::combine(frameList);
}

QString MotionBlur_sV::cachedFramePath(float framePos, FrameSize size)
{
    QString name = QString("%2/cached%1.png").arg(framePos, 8, 'f', 2, '0');
    if (size == FrameSize_Small) {
        name = name.arg(m_dirCacheSmall.absolutePath());
    } else if (size == FrameSize_Orig) {
        name = name.arg(m_dirCacheOrig.absolutePath());
    } else {
        qDebug() << "MotionBlur: Frame size " << toString(size) << " given, not supported!";
        Q_ASSERT(false);
    }
    if (!QFileInfo(name).exists()) {
        qDebug() << name << " does not exist yet. Interpolating.";
        QImage frm = m_project->interpolateFrameAt(framePos, size, InterpolationType_TwowayNew);
        frm.save(name);
    } else {
        qDebug() << "Frame is cached already: " << name;
    }
    return name;
}

void MotionBlur_sV::slotUpdateProjectDir()
{
    m_dirCacheSmall.rmdir(".");
    m_dirCacheOrig.rmdir(".");
    createDirectories();
}


void MotionBlur_sV::createDirectories()
{
    m_dirCacheSmall = m_project->getDirectory("cacheMotionBlurSmall");
    m_dirCacheOrig = m_project->getDirectory("cacheMotionBlurOrig");
}
