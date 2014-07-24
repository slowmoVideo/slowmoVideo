/*
 * class to export a movie using ffmpeg
*/
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QImage>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
    
#include "video_enc.h"
#include "ffmpeg_writer.h"
//#include "ffmpegEncode_sV.h"
#include "defs_sV.hpp"
    
QRegExp VideoFFMPEG::regexFrameNumber("frame=\\s*(\\d+)");

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
    //TODO: check this
#if 0
	return eatARGB(m_videoOut, frame.bits());
#else
    return 0;
#endif
}

int VideoFFMPEG::exportFrames(QString filepattern,RenderTask_sV *progress)
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

	process = new QProcess();
	process->start(settings.value("binaries/ffmpeg", "ffmpeg").toString(), args);
	if (!process->waitForStarted()) {
		qDebug() << "can't start encoding !";
		return 1;
	}

	// add a timer for reporting progress
	QTimer *m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressUpdate()));
    m_timer->start(100);
    
	process->waitForFinished();
	qDebug() << process->readAllStandardOutput();
    	qDebug() << process->readAllStandardError();
	process->terminate();
	qDebug() << process->exitStatus();

	m_timer->stop();
	delete m_timer;
	delete process;
	return 0;
}

/**
 *  Checks the progress of the ffmpeg threads by reading their stderr
 */
void VideoFFMPEG::slotProgressUpdate()
{
    QRegExp regex(regexFrameNumber);
    QString s;
   
    //m_ffmpegSemaphore.acquire();
    s = QString(process->readAllStandardError());
    if (regex.lastIndexIn(s) >= 0) {
    	qDebug() << "progress update " << regex.cap(1);
        //emit signalTaskProgress(regex.cap(1).toInt());
        //emit signalTaskItemDescription(tr("Frame %1 of %2").arg(regex.cap(1)).arg(m_videoInfo->framesCount));
    }
    //m_ffmpegSemaphore.release();
}


#pragma mark -
#pragma mark C bridge

VideoWriter* CreateVideoWriter_FFMPEG( const char* filename, int width, int height, double fps, const char *codec)
{
        VideoFFMPEG*  driver=  new VideoFFMPEG	(width,height,fps,codec,0,filename);
        return driver;
}

