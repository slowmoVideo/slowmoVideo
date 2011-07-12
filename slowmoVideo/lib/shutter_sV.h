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

/** \brief Simulates shutter (long exposure) with multiple images. */
class Shutter_sV
{
public:
    /// Combines the given images to a new image by addition and division.
    static QImage combine(QStringList images);
};

#endif // SHUTTER_SV_H
