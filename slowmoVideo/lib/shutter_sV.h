/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SHUTTER_SV_H
#define SHUTTER_SV_H

#include <QtGui/QImage>

class FlowField_sV;

/** \brief Simulates shutter (long exposure) with multiple images. */
class Shutter_sV
{
public:
    /// Combines the given images to a new image by addition and division.
    static QImage combine(const QStringList images);
    static QImage combine(const QList<QImage> images);

    static QImage convolutionBlur(const QImage source, const FlowField_sV *flow, float length);
    static QImage convolutionBlur(const QImage interpolatedAtOffset, const FlowField_sV *flow, float length, float offset);


private:

    struct ColorStack {
        ColorStack();
        void add(QColor col);
        QColor col();
    private:
        float r, g, b, a;
        int count;
    };

};

#endif // SHUTTER_SV_H
