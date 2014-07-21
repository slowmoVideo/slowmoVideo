/*
This file is part of slowmoVideo.
Copyright (C) 2014  Valery Brasseur <vbrasseur@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

/*
 * handle both ffmpeg and qtkit
 */
#ifndef EXPORTVIDEORENDERTARGET_SV_H
#define EXPORTVIDEORENDERTARGET_SV_H

#include "abstractRenderTarget_sV.h"

#include <QtCore/QDir>

class VideoWriter;
class RenderTask_sV;

/// Produces videos from frames.
// store temporary frame on disk (png) then export with "ffmpeg" or quicktime
class exportVideoRenderTarget : public AbstractRenderTarget_sV
{
public:
    /// Constructs a new video render target
    exportVideoRenderTarget(RenderTask_sV *parentRenderTask);
    virtual ~exportVideoRenderTarget();

    void openRenderTarget() throw(Error_sV) {} ;

    /// openRenderTarget() will throw an error if the target file cannot be opened.
    void setTargetFile(const QString& filename);
    /// Set a custom video codec
    void setVcodec(const QString& codec);

    void closeRenderTarget() throw(Error_sV);

	void setQT(int use) { use_qt = use;};
		
public slots:
    void slotConsumeFrame(const QImage &image, const int frameNumber);

private:
    QString m_filename;
    QString m_vcodec;
    VideoWriter* writer;

    int m_width;
    int m_height;
	int use_qt; // using QuickTime
    int first; // first frame to be written
	
    // png temp
    QDir m_targetDir;
    QString m_filenamePattern;
};

#endif // EXPORTVIDEORENDERTARGET_SV_H
