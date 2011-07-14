#ifndef PROJECTPREFERENCES_SV_H
#define PROJECTPREFERENCES_SV_H

#include "../lib/defs_sV.hpp"
class ProjectPreferences_sV
{
public:
    ProjectPreferences_sV();

    /**
      \return Reference to the previously selected tag axis
      */
    TagAxis& lastSelectedTagAxis();
    QPointF& viewport_t0();
    QPointF& viewport_secRes();

    // Rendering
    FrameSize& renderFrameSize();
    InterpolationType& renderInterpolationType();
    float& renderFPS();
    QString& imagesOutputDir();
    QString& imagesFilenamePattern();
    QString& videoFilename();

private:
    TagAxis m_tagAxis;
    QPointF m_viewport_t0;
    QPointF m_viewport_secRes;

    FrameSize m_renderFrameSize;
    InterpolationType m_renderInterpolationType;
    float m_renderFPS;

    QString m_imagesOutputDir;
    QString m_imagesFilenamePattern;
    QString m_videoFilename;

};

#endif // PROJECTPREFERENCES_SV_H
