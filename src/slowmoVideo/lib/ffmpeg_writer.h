/*
 * class to export a movie using ffmpeg
*/

#ifndef _FFMPEG_WRITER
#define _FFMPEG_WRITER

#include <QImage>

#include "video_enc.h"

// Against the «UINT64_C not declared» message.
// See: http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifdef __cplusplus
 #ifndef __STDC_CONSTANT_MACROS
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
 #endif // __STDC_CONSTANT_MACROS
#endif

extern "C" {
// ffmpeg libs
#include "../lib/ffmpegEncode_sV.h"
}

class VideoFFMPEG : public VideoWriter {
    int mHeight;
    int mWidth;
    double movieFPS;

	QString m_filename;
	QString m_vcodec;
    VideoOut_sV *m_videoOut;
    
public:
    VideoFFMPEG(int width,int height,double fps,const char *vcodec,const char* vquality,const char *filename);
    ~VideoFFMPEG();
    
    int writeFrame(const QImage& frame);

};

#endif // _FFMPEG_WRITER
