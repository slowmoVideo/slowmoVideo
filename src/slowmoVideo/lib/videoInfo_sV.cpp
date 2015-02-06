/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2014  Valery brasseur  <vbrasseur@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/


#include "videoInfo_sV.h"
#include "defs_sV.hpp"
#include <QtCore>
#include <QtCore/QSettings>
#include <QObject>

VideoInfoSV getInfo(const char filename[])
{
    VideoInfoSV info;
    info.frameRateNum = 0;
    info.frameRateDen = 0;
    info.streamsCount = -1;
    info.framesCount = 0;

    printf("Reading info for file %s.\n", filename);
    fflush(stdout);

    double videorate;
    double duration;
    QString output;
    
	QProcess ffmpeg;
        QSettings settings;
	QString prog = settings.value("binaries/ffmpeg", "ffmpeg").toString();
	QStringList args;
	args << "-i" << filename;
	
    ffmpeg.start(prog, args);
    ffmpeg.waitForFinished();
    QString videoInfo = ffmpeg.readAllStandardError();
    ffmpeg.close();
  
     // Example of output from 0.7 and up releases
    // Stream #0:0: Video: mpeg4 (Simple Profile) (DX50 / 0x30355844), yuv420p, 400x240 [SAR 1:1 DAR 5:3], 23 tbr, 23 tbn, 23 tbc    
    //  Duration: 00:00:11.13, start: 0.000000, bitrate: 6338 kb/s
    // Stream #0.0(eng): Video: mpeg4 (Main Profile), yuv420p, 1280x720 [PAR 1:1 DAR 16:9], 6309 kb/s, 30.69 fps, 90k tbn, 300 tbc

    qDebug() << "output : " << videoInfo;
    
    // find the source resolution
    //QRegExp rx("Stream.*Video:.*(([0-9]{2,5})x([0-9]{2,5}))");
    QRegExp rx("Stream.*Video:.*([1-9][0-9]*)x([1-9][0-9]*).*");
    //QRegExp rx("Stream.*Video:.*(\\d{2,})x(\\d{2,}).*");
    //rx.setMinimal(true);
    if (-1 == rx.indexIn(videoInfo)) {
        fprintf(stderr,"Could not find size.\n");
        return info;
    }

    info.width = rx.cap(1).toInt();
    info.height = rx.cap(2).toInt();
  
    // find the duration
    //rx.setPattern("Duration: (([0-9]+):([0-9]{2}):([0-9]{2}).([0-9]+))");
    rx.setPattern("Duration: ([0-9]*):([0-9]*):([0-9]*\\.[0-9]*)");
    if (-1 == rx.indexIn(videoInfo)) {
        fprintf(stderr,"Could not find duration of stream.\n");
        return info;
    }

    int hours = rx.cap(1).toInt();
    int minutes = rx.cap(2).toInt();
    double seconds = rx.cap(3).toDouble();
    
    duration = 3600*hours + 60*minutes + seconds;

    // container rate
    rx = QRegExp("([0-9\\.]+) fps");
    rx.setMinimal(true);
    if (rx.indexIn(videoInfo) !=-1)  {
        videorate = rx.cap(1).toDouble();
    } else {
	rx = QRegExp("Video:.*, ([0-9]*\\.?[0-9]+) tbn");
	rx.setMinimal(true);
	if (rx.indexIn(videoInfo) !=-1)  {
		videorate = rx.cap(1).toDouble();
	} else {
        videorate = 0;
    }
    }
    
    info.framesCount = duration * videorate;
    Fps_sV fps(videorate);

    info.frameRateNum = fps.num;
    info.frameRateDen = fps.den;

    info.streamsCount = 1;
    return info;
}
