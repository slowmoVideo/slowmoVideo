/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "imagesRenderTarget_sV.h"

#include <QtCore/QDebug>

ImagesRenderTarget_sV::ImagesRenderTarget_sV()
{
    m_targetDir = QDir::temp();
    m_filenamePattern = "rendered-%1.jpg";
}

void ImagesRenderTarget_sV::setTargetDir(const QDir dir)
{
    m_targetDir = dir;
}

bool ImagesRenderTarget_sV::setFilenamePattern(const QString pattern)
{
    if (pattern.contains("\\%1")) {
        m_filenamePattern = pattern;
        return true;
    }
    return false;
}

void ImagesRenderTarget_sV::slotConsumeFrame(const QImage &image, const int frameNumber)
{
    QString path = m_targetDir.absoluteFilePath(m_filenamePattern.arg(frameNumber+1, 5, 10, QChar::fromAscii('0')));

    bool ok;
    ok = image.save(path);
    if (!ok) {
        qDebug() << "Writing image to " << path << " failed!";
    }
}
