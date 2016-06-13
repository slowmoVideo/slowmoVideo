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

    Fps_sV& canvas_xAxisFPS();

    // Rendering
    QString& renderSectionMode();
    QString& renderStartTag();
    QString& renderEndTag();
    QString& renderStartTime();
    QString& renderEndTime();
    FrameSize& renderFrameSize();
    InterpolationType& renderInterpolationType();
    MotionblurType& renderMotionblurType();
    Fps_sV& renderFPS();
    QString& renderTarget();
    bool& renderFormat();
    QString& imagesOutputDir();
    QString& imagesFilenamePattern();
    QString& videoFilename();
    QString& videoCodec();

    float& flowV3DLambda();


private:
    TagAxis m_tagAxis;
    QPointF m_viewport_t0;
    QPointF m_viewport_secRes;

    Fps_sV m_canvas_xAxisFPS;

    QString m_renderSectionMode;
    QString m_renderStartTag;
    QString m_renderEndTag;
    QString m_renderStartTime;
    QString m_renderEndTime;
    FrameSize m_renderFrameSize;
    InterpolationType m_renderInterpolationType;
    MotionblurType m_motionblurType;
    Fps_sV m_renderFPS;
    QString m_renderTarget;
	bool m_renderFormat;
	
    QString m_imagesOutputDir;
    QString m_imagesFilenamePattern;
    QString m_videoFilename;
    QString m_vcodec;

    float m_flowV3DLambda;


};

#endif // PROJECTPREFERENCES_SV_H
