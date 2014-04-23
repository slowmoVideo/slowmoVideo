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
    virtual int writeFrame(const QImage* frame) = 0;
};

VideoWriter* CreateVideoWriter( const char* filename, int width,int height,double fps);
void ReleaseVideoWriter( VideoWriter** pwriter );
int WriteFrame( VideoWriter* writer, const QImage* frame);

/* lib dependant ... */

/*
VideoWriter* cvCreateVideoWriter_FFMPEG_proxy( const char* filename, int fourcc,
                                            double fps, CvSize frameSize, int is_color );
*/
VideoWriter* CreateVideoWriter_QT ( const char* filename, int width,int height,double fps);

#endif /* _VID_W_H */

