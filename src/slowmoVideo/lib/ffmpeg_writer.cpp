/*
 * class to export a movie using ffmpeg
*/

#include <QImage>

#include "video_enc.h"
#include "ffmpeg_writer.h"
//#include "ffmpegEncode_sV.h"
#include "defs_sV.hpp"
    
VideoFFMPEG::VideoFFMPEG(int width,int height,double fps,const char *vcodec,const char* vquality,const char *filename)
{
	m_videoOut = (VideoOut_sV*)malloc(sizeof(VideoOut_sV));
	
	m_filename = filename;
	m_vcodec = vcodec;
	
	Fps_sV m_fps(fps);
	
	char *pcodec = NULL;
    if (m_vcodec.length() > 0) {
        pcodec = (char*)malloc(m_vcodec.length()+1);
        strcpy(pcodec, m_vcodec.toStdString().c_str());
    }
    int worked =
    prepare(m_videoOut, m_filename.toStdString().c_str(), pcodec,
            width, height,
            fps * width * height,
            m_fps.den, m_fps.num);
    if (worked != 0) {
        //TODO: better here 
        fprintf(stderr,"cannot create FFMPEG encoder\n");
    }
}

VideoFFMPEG::~VideoFFMPEG()
{
	free(m_videoOut);
}

    
int VideoFFMPEG::writeFrame(const QImage& frame)
{
	return eatARGB(m_videoOut, frame.bits());
}


VideoWriter* CreateVideoWriter_FFMPEG( const char* filename, int width, int height, double fps)
{
        VideoFFMPEG*  driver=  new VideoFFMPEG	(width,height,fps,0,0,filename);
        return driver;
}

