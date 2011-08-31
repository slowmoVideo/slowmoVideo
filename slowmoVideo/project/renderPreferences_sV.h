#ifndef RENDERPREFERENCES_SV_H
#define RENDERPREFERENCES_SV_H

#include "../lib/defs_sV.hpp"

class RenderPreferences_sV
{
public:
    RenderPreferences_sV();

    InterpolationType interpolation;
    MotionblurType motionblur;
    FrameSize size;

    void setFps(Fps_sV fps);


    Fps_sV fps() const;

    bool fpsSetByUser() const;

private:
    Fps_sV m_fps;
    bool m_fpsSetByUser;
};

#endif // RENDERPREFERENCES_SV_H
