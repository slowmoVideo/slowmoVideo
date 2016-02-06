/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowField_sV.h"
#include "sourceField_sV.h"
#include "interpolate_sV.h"
#include "intMatrix_sV.h"
#include "shutter_sV.h"

#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include <algorithm>
#include <cmath>


#define CLAMP(x,min,max) (  ((x) < (min)) ? (min) : ( ((x) > (max)) ? (max) : (x) )  )

#define MIN_DIST 0.01


QImage Shutter_sV::combine(const QStringList images)
{
    Q_ASSERT(images.size() > 0);

    QImage img(images.at(0));
    IntMatrix_sV matrix(img.width(), img.height(), 4);
    for (int i = 0; i < images.size(); i++) {
        img = QImage(images.at(i));
        matrix += img.bits();
    }
    matrix /= images.size();

    unsigned char *bytes = matrix.toBytesArray();
    QImage result(matrix.width(), matrix.height(), QImage::Format_ARGB32);
    std::copy(bytes, bytes+matrix.width()*matrix.height()*matrix.channels()+1, result.bits());
    delete[] bytes;

    return result;
}

QImage Shutter_sV::combine(const QList<QImage> images)
{
    Q_ASSERT(images.size() > 0);

    IntMatrix_sV matrix(images.at(0).width(), images.at(0).height(), 4);
    for (int i = 0; i < images.size(); i++) {
        matrix += images.at(i).bits();
    }
    matrix /= images.size();

    unsigned char *bytes = matrix.toBytesArray();
    QImage result(matrix.width(), matrix.height(), QImage::Format_ARGB32);
    std::copy(bytes, bytes+matrix.width()*matrix.height()*matrix.channels()+1, result.bits());
    delete[] bytes;

    return result;
}

QImage Shutter_sV::convolutionBlur(const QImage source, const FlowField_sV *flow, float length)
{
    Q_ASSERT(source.width() == flow->width());
    Q_ASSERT(source.height() == flow->height());

    const float Wmax = source.width()-1.001;
    const float Hmax = source.height()-1.001;

    QImage blurred(source.size(), source.format());
    ColorStack stack;
    float dx, dy;
    float xf, yf;
    int samples, inc;
    for (int y = 0; y < source.height(); y++) {
        for (int x = 0; x < source.width(); x++) {
            stack = ColorStack();
            dx = length * flow->x(x,y);
            dy = length * flow->y(x,y);
            dx = CLAMP(x+dx, 0.0, Wmax)-x;
            dy = CLAMP(y+dy, 0.0, Hmax)-y;

            samples = ceil(std::sqrt(dx*dx + dy*dy));
            if (samples < 1) {
                samples = 1;
            }
            inc = std::max(1, samples/20); // Lower inc value leads to a smoother result

            xf = CLAMP(x, 0.0, Wmax);
            yf = CLAMP(y, 0.0, Hmax);
            stack.add(source.pixel(x,y)); // Avoids interpolation error, and interpolation for (x,y) is not necessary anyway
            for (int i = 1; i <= samples; i += inc) {
                // \todo adjust increment
                stack.add(Interpolate_sV::interpolate(source, xf+float(i)/samples * dx, yf+float(i)/samples * dy));
            }
            blurred.setPixel(x, y, stack.col().rgba());
        }
    }
    return blurred;
}

QImage Shutter_sV::convolutionBlur(const QImage interpolatedAtOffset, const FlowField_sV *flow, float length, float offset)
{
    Q_ASSERT(interpolatedAtOffset.width() == flow->width());
    Q_ASSERT(interpolatedAtOffset.height() == flow->height());
    // could be equal to 0, in case of integer !
    //Q_ASSERT(offset > 0);
    Q_ASSERT(offset < 1);

    SourceField_sV source(flow, offset);
    source.inpaint();

    const float Wmax = interpolatedAtOffset.width()-1.01;
    const float Hmax = interpolatedAtOffset.height()-1.01;

    QImage blurred(interpolatedAtOffset.size(), interpolatedAtOffset.format());
    ColorStack stack;
    float dx, dy;
    float xf, yf;
    int samples, inc;
    for (int y = 0; y < interpolatedAtOffset.height(); y++) {
        for (int x = 0; x < interpolatedAtOffset.width(); x++) {
            stack = ColorStack();
            dx = -(source.at(x,y).fromX - x); // Get the optical flow vector back from the source field
            dy = -(source.at(x,y).fromY - y);
            dx = dx/offset * length; // First normalize to one frame, then adjust the length
            dy = dy/offset * length;
            dx = CLAMP(x+dx, 0.0, Wmax)-x;
            dy = CLAMP(y+dy, 0.0, Hmax)-y;

            samples = ceil(std::sqrt(dx*dx + dy*dy));
            if (samples < 1) {
                samples = 1;
            }
            inc = std::max(1, samples/20);

            xf = CLAMP(x, 0.0, Wmax);
            yf = CLAMP(y, 0.0, Hmax);
            stack.add(interpolatedAtOffset.pixel(x,y)); // Avoids interpolation error, and interpolation for (x,y) is not necessary anyway
            for (int i = 1; i <= samples; i += inc) {
                // \todo adjust increment
                stack.add(Interpolate_sV::interpolate(interpolatedAtOffset, xf+float(i)/samples * dx, yf+float(i)/samples * dy));
            }
            blurred.setPixel(x, y, stack.col().rgba());
        }
    }
    return blurred;

}

Shutter_sV::ColorStack::ColorStack() :
    r(0), g(0), b(0), a(0), count(0)
{}

void Shutter_sV::ColorStack::add(QColor col)
{
    r += col.redF();
    g += col.greenF();
    b += col.blueF();
    a += col.alphaF();
    ++count;
}

QColor Shutter_sV::ColorStack::col()
{
    return QColor::fromRgbF(r/count, g/count, b/count, a/count);
}
