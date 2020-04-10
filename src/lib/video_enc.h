/*
 * global abstract file
 */

#ifndef _VID_W_H
#define _VID_W_H

#include <QImage>

class RenderTask_sV;

class VideoWriter
{
public:
    virtual ~VideoWriter() {};
    virtual int writeFrame(const QImage& frame) = 0;
    virtual int exportFrames(QString filepattern,int first,RenderTask_sV* progress) = 0;
};

VideoWriter* CreateVideoWriter( const char* filename, int width,int height,double fps,int use_qt,const char* codec);
void ReleaseVideoWriter( VideoWriter** pwriter );
int WriteFrame( VideoWriter* writer, const QImage& frame);
int exportFrames(VideoWriter* pwriter,QString filepattern,int first,RenderTask_sV* progress);

/* lib dependant ... */


VideoWriter* CreateVideoWriter_FFMPEG(const char* filename, int width,int height,double fps,const char* codec);

VideoWriter* CreateVideoWriter_QT ( const char* filename, int width,int height,double fps,const char* codec);

#endif /* _VID_W_H */

