#include "config.h"

#include "video_enc.h"

VideoWriter* CreateVideoWriter( const char* filename, int width, int height,double fps,int use_qt)
{
	VideoWriter* driver;
	
#ifdef USE_QTKIT
	if (use_qt)
		driver= CreateVideoWriter_QT(filename,width, height,fps);
	else
#endif
#ifdef USE_FFMPEG
		driver= CreateVideoWriter_FFMPEG(filename,width, height,fps);
#endif
	return driver;
}

int WriteFrame( VideoWriter* writer, const QImage& frame)
{
    return writer ? writer->writeFrame(frame) : 0;
}

void ReleaseVideoWriter( VideoWriter** pwriter )
{
    if( pwriter && *pwriter ) {
        delete *pwriter;
        *pwriter = 0;
    }
}
