#ifndef ABSTRACTFRAMESOURCE_SV_H
#define ABSTRACTFRAMESOURCE_SV_H

#include "../lib/defs_sV.h"
#include "../lib/defs_sV.hpp"

#include <QImage>
#include <QtCore/QDir>

/** Represents a source for input frames, like a video or an image sequence */
class AbstractFrameSource_sV
{
public:
    AbstractFrameSource_sV(const QDir &projectDir);
    virtual ~AbstractFrameSource_sV();

    virtual void init();

    const VideoInfoSV& videoInfo() const { return *m_videoInfo; }

    /**
      @return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    virtual QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) const;

    virtual QImage interpolateFrameAt(float time, const FrameSize frameSize) const;

protected:
    /** Needs to be initialized! */
    VideoInfoSV *m_videoInfo;
    QDir m_projectDir;
};

#endif // ABSTRACTFRAMESOURCE_SV_H
