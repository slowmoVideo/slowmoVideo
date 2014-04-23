#include "video_enc.h"

VideoWriter* CreateVideoWriter( const char* filename, int width, int height,double fps)
{
	VideoWriter* driver= CreateVideoWriter_QT(filename,width, height,fps);
	return driver;
}

int WriteFrame( VideoWriter* writer, const QImage* frame)
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
