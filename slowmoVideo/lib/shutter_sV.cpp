/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "shutter_sV.h"
#include "intMatrix_sV.h"

#include <QtCore/QStringList>

QImage Shutter_sV::combine(QStringList images)
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
    QImage result(bytes, matrix.width(), matrix.height(), QImage::Format_ARGB32);
    delete bytes;

    return result;
}
