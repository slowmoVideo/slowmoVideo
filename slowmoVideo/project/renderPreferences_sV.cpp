#include "renderPreferences_sV.h"


RenderPreferences_sV::RenderPreferences_sV() :
    interpolation(InterpolationType_TwowayNew),
    motionblur(MotionblurType_Convolving),
    size(FrameSize_Orig),
    m_fps(24),
    m_fpsSetByUser(false)
{
}

Fps_sV RenderPreferences_sV::fps() const
{
    return m_fps;
}

void RenderPreferences_sV::setFps(Fps_sV fps)
{
    Q_ASSERT(fps.num > 0);
    m_fpsSetByUser = true;
    m_fps = fps;
}

bool RenderPreferences_sV::fpsSetByUser() const
{
    return m_fpsSetByUser;
}
