#ifndef ABSTRACTFRAMESOURCE_SV_H
#define ABSTRACTFRAMESOURCE_SV_H

#include "../lib/defs_sV.hpp"

#include <QImage>
#include <QtCore/QDir>
#include <inttypes.h>
class Project_sV;

class Div0Exception
{

};

/** Represents a source for input frames, like a video or an image sequence */
class AbstractFrameSource_sV
{
public:
    AbstractFrameSource_sV(const Project_sV *project);
    virtual ~AbstractFrameSource_sV();

    virtual int64_t framesCount() const = 0;
    virtual int frameRateNum() const = 0;
    virtual int frameRateDen() const = 0;
    float fps() const throw(Div0Exception);

    /**
      @return The frame at the given position, as image. Fails
      if the frames have not been extracted yet.
      */
    virtual QImage frameAt(const uint frame, const FrameSize frameSize = FrameSize_Orig) const = 0;

protected:
    const Project_sV *m_project;

};


#endif // ABSTRACTFRAMESOURCE_SV_H
