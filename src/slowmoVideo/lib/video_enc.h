/*
 * global abstract file
 */

#ifndef _VID_W_H
#define _VID_W_H

#include <QImage>

class VideoWriter
{
public:
    virtual ~VideoWriter() {};
    virtual int writeFrame(const QImage& frame) = 0;
};

VideoWriter* CreateVideoWriter( const char* filename, int width,int height,double fps,int use_qt);
void ReleaseVideoWriter( VideoWriter** pwriter );
int WriteFrame( VideoWriter* writer, const QImage& frame);

/* lib dependant ... */


VideoWriter* CreateVideoWriter_FFMPEG(const char* filename, int width,int height,double fps);

VideoWriter* CreateVideoWriter_QT ( const char* filename, int width,int height,double fps);

#endif /* _VID_W_H */

