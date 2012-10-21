/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "canvasTools.h"

#include "canvas.h"
#include "project/project_sV.h"
#include "project/projectPreferences_sV.h"

//#define DEBUG
#ifdef DEBUG
#include <QDebug>
#endif

QString CanvasTools::outputTimeLabel(Canvas *canvas, Node_sV &time)
{
    int decimals = 0;
    float maxRes = 1;

    while (canvas->m_secResX > maxRes) {
        decimals++;
        maxRes *= 10;
    }

#ifdef DEBUG
    qDebug() << "resX: " << canvas->m_secResX << ", decimals: " << decimals << ", max res: " << maxRes;
#endif

    QString timeText;
    if (time.x() < 60) {
        timeText = QString("%1 s").arg(time.x(), 0, 'f', decimals);
    } else {
        timeText = QString("%1 min %2 s").arg(int(time.x()/60)).arg(time.x()-60*int(time.x()/60), 0, 'f', decimals);
    }

    float frame = canvas->m_project->preferences()->canvas_xAxisFPS().fps()*time.x();
    timeText += QString("\nFrame %1").arg(frame, 0, 'f', (decimals <= 1 ? 0 : 1));

    return timeText;
}

QString CanvasTools::outputSpeedLabel(Node_sV &time, Project_sV *project)
{
    if (!project->nodes()->isInsideCurve(time.x())) {
        return "";
    }

    const qreal dx = 1.0/project->preferences()->canvas_xAxisFPS().fps();


    qreal t1, t2;
    if (time.x()+dx <= project->nodes()->endTime()) {
        t1 = project->nodes()->sourceTime(time.x());
        t2 = project->nodes()->sourceTime(time.x()+dx);
    } else {
        t1 = project->nodes()->sourceTime(time.x()-dx);
        t2 = project->nodes()->sourceTime(time.x());
    }

    const qreal dy = t2-t1;

    qreal percent = 0;
    if (dy != 0) {
        percent = dy/dx;
    }

    return QString("%1 %").arg(percent, 0, 'f');//, 1);


}
