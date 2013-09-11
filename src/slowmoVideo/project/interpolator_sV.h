#ifndef INTERPOLATOR_SV_H
#define INTERPOLATOR_SV_H

#include "renderPreferences_sV.h"
#include "project_sV.h"

class Interpolator_sV
{
public:
    static QImage interpolate(Project_sV *project, float frame, const RenderPreferences_sV& prefs)
                             throw(FlowBuildingError, InterpolationError);
};

#endif // INTERPOLATOR_SV_H
