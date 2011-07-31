/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "sourceField_sV.h"
#include "flowField_sV.h"
#include <cmath>

#define FIX_FLOW

SourceField_sV::SourceField_sV(int width, int height) :
    m_width(width),
    m_height(height)
{
    m_field = new Source[width*height];
}

SourceField_sV::SourceField_sV(const FlowField_sV *flow, float pos) :
    m_width(flow->width()),
    m_height(flow->height())
{
    m_field = new Source[m_width*m_height];

    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {

            float tx = x + pos * flow->x(x,y);
            float ty = y + pos * flow->y(x,y);

            // +.5: Round to nearest
            int ix = floor(tx+.5);
            int iy = floor(ty+.5);

            // The position the pixel moved to is a float, but to avoid very complex
            // interpolation (how to set a pixel at (55.3, 97.16) to red?), this information
            // is reverted (where did (55, 97) come from? -> (50.8, 101.23) which can be
            // interpolated easily from the source image)
            if (ix >= 0 && iy >= 0 &&
                    ix < m_width && iy < m_height) {
                at(ix, iy).set(x + (ix-tx), y + (iy-ty));
            }

        }
    }
}

SourceField_sV::~SourceField_sV()
{
    delete[] m_field;
}

void SourceField_sV::inpaint()
{
    SourceSum sum;
    int dist;
    bool xm, xp, ym, yp;

    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            if (!at(x,y).isSet) {
                sum.reset();
                dist = 1;
                while (sum.count <= 2) {
                    xm = (x-dist) >= 0;
                    xp = (x+dist) < m_width;
                    ym = (y-dist) >= 0;
                    yp = (y+dist) < m_height;

                    if (xm) {
                        sum += at(x-dist, y);
                    }
                    if (ym) {
                        sum += at(x, y-dist);
                    }
                    if (xp) {
                        sum += at(x+dist, y);
                    }
                    if (yp) {
                        sum += at(x, y+dist);
                    }
                    if (sum.count > 2) break;

                    if (xm) {
                        if (ym) {
                            sum += at(x-dist, y-dist);
                        }
                        if (yp) {
                            sum += at(x-dist, y+dist);
                        }
                    }
                    if (xp) {
                        if (ym) {
                            sum += at(x+dist, y-dist);
                        }
                        if (yp) {
                            sum += at(x+dist, y+dist);
                        }
                    }
                    dist++;
                }
                at(x,y) = sum.norm();
            }
        }
    }
}
