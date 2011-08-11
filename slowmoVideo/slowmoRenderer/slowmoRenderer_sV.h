/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SLOWMORENDERER_SV_H
#define SLOWMORENDERER_SV_H

#include "lib/defs_sV.hpp"
#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <string>

class Project_sV;

class Error {
public:
    Error(std::string message);
    std::string message;
};

/**
  \brief Just for rendering.
  */
class SlowmoRenderer_sV : public QObject
{
    Q_OBJECT
public:
    SlowmoRenderer_sV();
    ~SlowmoRenderer_sV();

    void load(QString filename) throw(Error);
    void start();
    void abort();

    void setTimeRange(QString start, QString end);
    void setFps(double fps);
    void setVideoRenderTarget(QString filename, QString codec);
    void setImagesRenderTarget(QString filenamePattern, QString directory);
    void setInterpolation(InterpolationType interpolation);
    void setSize(bool original);
    void setV3dLambda(float lambda);


    void printProgress();

    /// Checks if all necessary parameters (e.g. paths) are set
    /// \param message Will contain an error message if the function returned \c false
    bool isComplete(QString &message) const;


private:
    Project_sV *m_project;

    int m_taskSize;
    int m_lastProgress;

    QString m_start;
    QString m_end;

    bool m_renderTargetSet;


private slots:
    void slotProgressInfo(int progress);
    void slotTaskSize(QString desc, int size);
    void slotFinished(QString time);
};


#endif // SLOWMORENDERER_SV_H
