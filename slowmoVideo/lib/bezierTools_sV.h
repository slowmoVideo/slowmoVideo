/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef BEZIERTOOLS_SV_H
#define BEZIERTOOLS_SV_H

#include "defs_sV.hpp"

class BezierTools_sV
{
public:
    static QPointF interpolateAtX(float x, QPointF p0, QPointF p1, QPointF p2, QPointF p3);
    static QPointF interpolate(float t, QPointF p0, QPointF p1, QPointF p2, QPointF p3);
};

#endif // BEZIERTOOLS_SV_H
