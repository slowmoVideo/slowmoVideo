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
// for reporting
#include "../project/renderTask_sV.h"
    
QRegExp VideoFFMPEG::regexFrameNumber("frame=\\s*(\\d+)");

VideoFFMPEG::VideoFFMPEG(int width,int height,double fps,const char *vcodec,const char* vquality,const char *filename)
{
	m_videoOut = (VideoOut_sV*)malloc(sizeof(VideoOut_sV));
	
	m_filename = strdup(filename);
	if (vcodec != 0)
		m_vcodec = strdup(vcodec);
	else 
		m_vcodec = strdup("libx264"); // -b:v 5000k
	
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
    process = 0;
}

VideoFFMPEG::~VideoFFMPEG()
{
	//TODO:
	//m_dirFramesOrig.rmdir(".");
	if (process != 0) {
		if (process->state() == QProcess::Running) {
			abort();
			process->waitForFinished();
		}
	}
	free(m_vcodec);
	free(m_filename);
	free(m_videoOut);
}

#pragma mark -
    
int VideoFFMPEG::writeFrame(const QImage& frame)
{
    //TODO: check this
#if 0
	return eatARGB(m_videoOut, frame.bits());
#else
	return (-1);
#endif
}

int VideoFFMPEG::exportFrames(QString filepattern,int first,RenderTask_sV *progress)
{
	QSettings settings;

	qDebug() << "exporting frame from : " << filepattern << " to " << m_filename;

	QStringList args;

	args << "-r" << QString::number(movieFPS);
	args << "-f" << "image2";
	if (first != 0)
		args << "-start_number" << QString::number(first);
	args << "-i" << filepattern;
	args << "-vcodec" << m_vcodec;
	args << "-s" << QString("%1x%2").arg(QString::number(mWidth), QString::number(mHeight));
	args << m_filename;
   
        qDebug() << "Arguments: " << args;

	this->progress = progress;
	last = 0;
	process = new QProcess;
	//QObject::connect(process, SIGNAL(started()), this, SLOT(processStarted()));
	QObject::connect(process, SIGNAL(finished(int)), this, SLOT(encodingFinished(int)));
	QObject::connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(readOutput()));
	QObject::connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readOutput()));
  QObject::connect(process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(ffmpegError(QProcess::ProcessError)));

  QObject::connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(process_state_changed()));

	process->start(settings.value("binaries/ffmpeg", "ffmpeg").toString(), args);
	if (!process->waitForStarted()) {
		qDebug() << "can't start encoding !";
	  process->deleteLater();
	  process = 0;
		return 1;
	}

	// warn: default timeout at 30s !
	process->waitForFinished(-1); // let time goes on !
	qDebug() << process->readAllStandardOutput();
    	qDebug() << process->readAllStandardError();
	process->terminate();
	qDebug() << "exit : " << process->exitStatus();

	delete process;
	process = 0;
	return 0;
}

#pragma mark -
#pragma mark C bridge
void VideoFFMPEG::process_state_changed()
{
    if (process->state() == QProcess::Starting) {
        qDebug() << "Process is starting up...";
    }
    if (process->state() == QProcess::Running) {
        qDebug() << "Process is now running.";
    }
    if (process->state() == QProcess::NotRunning) {
        qDebug() << "Process is finished running.";
    }
}

void VideoFFMPEG::processStarted()
{
	qDebug() << "process started";
}

void VideoFFMPEG::readOutput()
{
	QRegExp regex(regexFrameNumber);
   
	//qDebug() << "process read";
	QString line = process->readAllStandardOutput();
	//qDebug() << "got [" << line << "]";
	line = process->readAllStandardError();
 
    	if (regex.lastIndexIn(line) >= 0) {
        	//emit signalTaskProgress(;);
		//qDebug() << "prog update : " << regex.cap(1).toInt();
		progress->stepProgress(regex.cap(1).toInt()-last);
		last = regex.cap(1).toInt();
    	}
	//qDebug() << "got " << line;
    //TODO: may check if we need to stop/cancel here
    // qprocess->kill() ?
}

void VideoFFMPEG::ffmpegError(QProcess::ProcessError error)
{
				qDebug() << "ffmpeg finish with error : " << error;
}

void VideoFFMPEG::encodingFinished(int error) 
{
	if (error != 0)
		qDebug() << "process finish with error : " << error;
}

VideoWriter* CreateVideoWriter_FFMPEG( const char* filename, int width, int height, double fps, const char *codec)
{
        VideoFFMPEG*  driver=  new VideoFFMPEG	(width,height,fps,codec,0,filename);
        return driver;
}

