/*
 * class to export a movie using ffmpeg
*/

#include <QImage>

#include "video_enc.h"

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

