#ifndef INTERPOLATOR_SV_H
#define INTERPOLATOR_SV_H

#include "../lib/defs_sV.hpp"
#include "project_sV.h"

class Interpolator_sV
{
public:
    static QImage interpolate(Project_sV *project, float frame, InterpolationType interpolation, FrameSize size)
                             throw(FlowBuildingError, InterpolationError);
};

#endif // INTERPOLATOR_SV_H
