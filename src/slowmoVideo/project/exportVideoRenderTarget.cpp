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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QTemporaryDir> 
#endif

exportVideoRenderTarget::exportVideoRenderTarget(RenderTask_sV *parentRenderTask) :
    AbstractRenderTarget_sV(parentRenderTask)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QTemporaryDir tempDir("slowmovideo");
    if (tempDir.isValid()) 
    	m_targetDir = QDir(tempDir.path());
    else 
#endif
     m_targetDir = QDir::temp();

    m_filenamePattern = "rendered-%1.png";
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

void exportVideoRenderTarget::exportRenderTarget() throw(Error_sV)
{
	qDebug() << "exporting temporary frame to Video";
	openRenderTarget();
// loop throught frame ?
	closeRenderTarget();
}


