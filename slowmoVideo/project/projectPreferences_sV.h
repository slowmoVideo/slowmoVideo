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
    QString& renderSectionMode();
    QString& renderStartTag();
    QString& renderEndTag();
    QString& renderStartTime();
    QString& renderEndTime();
    FrameSize& renderFrameSize();
    InterpolationType& renderInterpolationType();
    Fps_sV& renderFPS();
    QString& renderTarget();
    QString& imagesOutputDir();
    QString& imagesFilenamePattern();
    QString& videoFilename();
    QString& videoCodec();

    float& flowV3DLambda();

private:
    TagAxis m_tagAxis;
    QPointF m_viewport_t0;
    QPointF m_viewport_secRes;

    QString m_renderSectionMode;
    QString m_renderStartTag;
    QString m_renderEndTag;
    QString m_renderStartTime;
    QString m_renderEndTime;
    FrameSize m_renderFrameSize;
    InterpolationType m_renderInterpolationType;
    Fps_sV m_renderFPS;
    QString m_renderTarget;

    QString m_imagesOutputDir;
    QString m_imagesFilenamePattern;
    QString m_videoFilename;
    QString m_vcodec;

    float m_flowV3DLambda;

};

#endif // PROJECTPREFERENCES_SV_H
