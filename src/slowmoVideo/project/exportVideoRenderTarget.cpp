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

#include "renderTask_sV.h"
#include <QtCore/QObject>

#include "../lib/video_enc.h"

exportVideoRenderTarget::exportVideoRenderTarget(RenderTask_sV *parentRenderTask) :
    AbstractRenderTarget_sV(parentRenderTask)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    //QTemporaryDir tempDir("slowmovideo");
    QTemporaryDir tempDir;; // use default
    if (tempDir.isValid()) 
    	m_targetDir = QDir(tempDir.path());
    else 
#endif
     m_targetDir = QDir::temp();

qDebug() << "  target dir " << m_targetDir;

    m_filenamePattern = "rendered-%1.png";
    
    use_qt = 1;
}

exportVideoRenderTarget::~exportVideoRenderTarget()
{
	// QT bug ?
 	m_targetDir.rmdir(".");
}

void exportVideoRenderTarget::setTargetFile(const QString &filename)
{
    m_filename = filename;
}
void exportVideoRenderTarget::setVcodec(const QString &codec)
{
    m_vcodec = codec;
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

void exportVideoRenderTarget::closeRenderTarget() throw(Error_sV)
{	
	VideoWriter* writer;;

	qDebug() << "exporting temporary frame to Video" << m_filename << " using codec " << m_vcodec;
	writer = CreateVideoWriter(m_filename.toStdString().c_str(),
    		renderTask()->resolution().width(),
    		renderTask()->resolution().height(),
    		renderTask()->fps().fps(),use_qt,m_vcodec.toStdString().c_str());
    
   
    if (writer == 0) {
        throw Error_sV(QObject::tr("Video could not be prepared .\n"));
    }
	// loop throught frame ?
	// TODO: 
	exportFrames(writer, m_targetDir.absoluteFilePath(m_filenamePattern.arg("%05d")).toStdString().c_str());
	ReleaseVideoWriter( &writer );
}

  



