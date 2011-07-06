#include "projectPreferences_sV.h"
#include <QtCore/QDir>

ProjectPreferences_sV::ProjectPreferences_sV() :
    m_tagAxis(TagAxis_Source),
    m_viewport_t0(0, 0),
    m_viewport_secRes(50, 50),
    m_renderFrameSize(FrameSize_Orig),
    m_renderFPS(24),
    m_imagesOutputDir(QDir::homePath()),
    m_imagesFilenamePattern("rendered-%1.jpg")
{
}

TagAxis& ProjectPreferences_sV::lastSelectedTagAxis()
{
    return m_tagAxis;
}

QPointF& ProjectPreferences_sV::viewport_secRes()
{
    return m_viewport_secRes;
}
QPointF& ProjectPreferences_sV::viewport_t0()
{
    return m_viewport_t0;
}
FrameSize& ProjectPreferences_sV::renderFrameSize()
{
    return m_renderFrameSize;
}
float& ProjectPreferences_sV::renderFPS()
{
    return m_renderFPS;
}

QString& ProjectPreferences_sV::imagesOutputDir()
{
    return m_imagesOutputDir;
}
QString& ProjectPreferences_sV::imagesFilenamePattern()
{
    return m_imagesFilenamePattern;
}
