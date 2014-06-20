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
	
	m_filename = strdup(filename);
	if (vcodec != 0)
		m_vcodec = strdup(vcodec);
	else 
		m_vcodec = 0;
	
	Fps_sV m_fps(fps);
	movieFPS = fps;
#if 0	
	char *pcodec = NULL;
    if (m_vcodec.length() > 0) {
        pcodec = (char*)malloc(m_vcodec.length()+1);
        strcpy(pcodec, m_vcodec.toStdString().c_str());
    }

    int worked =
    prepare(m_videoOut, m_filename, pcodec,
            width, height,
            fps * width * height,
            m_fps.den, m_fps.num);
    if (worked != 0) {
        //TODO: better here 
        fprintf(stderr,"cannot create FFMPEG encoder\n");
    }
#endif
}

VideoFFMPEG::~VideoFFMPEG()
{
	free(m_vcodec);
	free(m_filename);
	free(m_videoOut);
}

    
int VideoFFMPEG::writeFrame(const QImage& frame)
{
	return eatARGB(m_videoOut, frame.bits());
}

int VideoFFMPEG::exportFrames(const char* filepattern)
{
	char exec_cmd[1024];

	fprintf(stderr,"exporting frame from [%s] to %s\n",filepattern,m_filename);
	// should use preferences for getting ffmpeg path
	// TODO: more args parameters :
	// fps, w x h
        // codec ...
	snprintf(exec_cmd,sizeof(exec_cmd),
		"ffmpeg -f image2 -i \"%s\" -r %f -vcodec libx264 -b:v 5000k -s 1920×1080 %s",
		filepattern,movieFPS,
		m_filename);
#if 0
QProcess process;
process.start("gedit", QStringList() << "/home/oDx/Documents/a.txt");
#endif

	fprintf(stderr,"command is : %s\n",exec_cmd);
	return 0;
}


VideoWriter* CreateVideoWriter_FFMPEG( const char* filename, int width, int height, double fps)
{
        VideoFFMPEG*  driver=  new VideoFFMPEG	(width,height,fps,0,0,filename);
        return driver;
}

