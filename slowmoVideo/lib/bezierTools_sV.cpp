/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "bezierTools_sV.h"

#include <cmath>
#include <iostream>

SimplePointF_sV BezierTools_sV::interpolateAtX(float x, SimplePointF_sV p0, SimplePointF_sV p1, SimplePointF_sV p2, SimplePointF_sV p3)
{
    float delta = 1;
    float t = 0;
    int iterations = 10*(p3.x-p0.x);

    for (int i = 0; i < iterations; i++) {
        float plus  = interpolate(t+delta, p0, p1, p2, p3).x;
        float minus = interpolate(t-delta, p0, p1, p2, p3).x;
        float norm  = interpolate(t      , p0, p1, p2, p3).x;
        if ((t+delta) <= 1 && fabs(plus-x) < fabs(norm-x)) {
            t += delta;
        } else if ((t-delta) >= 0 && fabs(minus-x) < fabs(norm-x)) {
            t -= delta;
        }
        delta /= 2;
    }
//    std::cout << "Interpolating at t=" << t << " for x time " << 100*interpolate(t, p0, p1, p2, p3).x << ": " << interpolate(t, p0, p1, p2, p3).y << std::endl;
    return interpolate(t, p0, p1, p2, p3);
}

SimplePointF_sV BezierTools_sV::interpolate(float t, SimplePointF_sV p0, SimplePointF_sV p1, SimplePointF_sV p2, SimplePointF_sV p3)
{
    p0 = p0 * 1 * pow(t,0) * pow(1-t, 3);
    p1 = p1 * 3 * pow(t,1) * pow(1-t, 2);
    p2 = p2 * 3 * pow(t,2) * pow(1-t, 1);
    p3 = p3 * 1 * pow(t,3) * pow(1-t, 0);
    return p0+p1+p2+p3;
}
