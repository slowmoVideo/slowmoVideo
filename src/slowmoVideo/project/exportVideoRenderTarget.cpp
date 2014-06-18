/*
This file is part of slowmoVideo.
Copyright (C) 2014 Valery Brasseur <vbrasseur@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "exportVideoRenderTarget.h"

#include <QtCore/QDebug>
//TODO QT5?
#include <QTemporaryDir> 

exportVideoRenderTarget::exportVideoRenderTarget(RenderTask_sV *parentRenderTask) :
    AbstractRenderTarget_sV(parentRenderTask)
{
//TODO: better ?
#if 1
    QTemporaryDir tempDir("slowmovideo");
    if (tempDir.isValid()) 
    	m_targetDir = QDir(tempDir.path());
    else 
#endif
     m_targetDir = QDir::temp();

    m_filenamePattern = "rendered-%1.png";
}

void exportVideoRenderTarget::setTargetDir(const QDir dir)
{
    m_targetDir = dir;
}

bool exportVideoRenderTarget::setFilenamePattern(const QString pattern)
{
//TODO: 
#if 0
    if (pattern.contains("%1")) {
        m_filenamePattern = pattern;
        return true;
    }
    return false;
#else
    return true;
#endif
}

void exportVideoRenderTarget::slotConsumeFrame(const QImage &image, const int frameNumber)
{
    if (!m_targetDir.exists()) {
        m_targetDir.mkpath(".");
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString path = m_targetDir.absoluteFilePath(m_filenamePattern.arg(frameNumber+1, 5, 10, QChar::fromAscii('0')));
#else
    QString path = m_targetDir.absoluteFilePath(m_filenamePattern.arg(frameNumber+1, 5, 10, QChar::fromLatin1('0')));
#endif

    bool ok;
    ok = image.save(path);
    if (!ok) {
        qDebug() << "  Writing image to " << path << " failed!";
    } else {
        qDebug() << "  Saved frame number " << frameNumber << " to " << path;
    }
}

