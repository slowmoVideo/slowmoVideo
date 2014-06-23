/*
 * class to export a movie using ffmpeg
*/
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
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
		m_vcodec = strdup("libx264 -b:v 5000k ");;
	
	Fps_sV m_fps(fps);
	movieFPS = fps;
	mHeight = height;
	mWidth = width;

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
	//TODO:
	//m_dirFramesOrig.rmdir(".");
	free(m_vcodec);
	free(m_filename);
	free(m_videoOut);
}

    
int VideoFFMPEG::writeFrame(const QImage& frame)
{
	return eatARGB(m_videoOut, frame.bits());
}

int VideoFFMPEG::exportFrames(QString filepattern)
{
	QSettings settings;

	qDebug() << "exporting frame from : " << filepattern << " to " << m_filename;

	QStringList args;

	args << "-f" << "image2";
	args << "-i" << filepattern;
	args << "-r" << QString::number(movieFPS);
	args << "-vcodec" << m_vcodec;
	args << "-s" << QString("%1x%2").arg(QString::number(mWidth), QString::number(mHeight));
	args << m_filename;
   
        qDebug() << "Arguments: " << args;

	QProcess process;
	process.start(settings.value("binaries/ffmpeg", "ffmpeg").toString(), args);
	if (!process.waitForStarted()) {
		qDebug() << "can't start encoding !";
		return 1;
	}

	process.waitForFinished();
	qDebug() << process.readAllStandardOutput();
    	qDebug() << process.readAllStandardError();
	process.terminate();
	qDebug() << process.exitStatus();

	return 0;
}


VideoWriter* CreateVideoWriter_FFMPEG( const char* filename, int width, int height, double fps, const char *codec)
{
        VideoFFMPEG*  driver=  new VideoFFMPEG	(width,height,fps,codec,0,filename);
        return driver;
}

