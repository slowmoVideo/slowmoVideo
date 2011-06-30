/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef PROJECT_SV_H
#define PROJECT_SV_H

#include "tag_sV.h"
#include "../lib/defs_sV.hpp"
extern "C" {
#include "../lib/videoInfo_sV.h"
}

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include <QSettings>

class Flow_sV;
class AbstractFrameSource_sV;
class AbstractFlowSource_sV;
class RenderTask_sV;
class NodeList_sV;
class FlowField_sV;
class QSignalMapper;
class QProcess;
class QRegExp;
class QTimer;
class Project_sV : public QObject
{
    Q_OBJECT

public:
    /** Creates an empty project. A video file can be loaded with loadFile(QString, QString). */
    Project_sV();
    /**
      Creates a new project.
      @param filename Input video file
      @param projectDir Project directory; All cached files will be put in there.
      */
    Project_sV(QString projectDir);
    ~Project_sV();

    void setProjectDir(QString projectDir);
    /** The project filename should be set when saving or loading the project. */
    void setProjectFilename(QString filename);
    /** \return The filename this project was last saved as. */
    QString projectFilename() const;

    void readSettings(const QSettings &settings);

    /** \param frameSource will be managed (deleted) by the project. If \c NULL, an empty frame source will be used. */
    void loadFrameSource(AbstractFrameSource_sV *frameSource);

    AbstractFrameSource_sV* frameSource() { return m_frameSource; }
    AbstractFlowSource_sV* flowSource() { return m_flowSource; }
    NodeList_sV *nodes() const { return m_nodes; }
    QList<Tag_sV> *tags() const { return m_tags; }

    RenderTask_sV *renderTask() { return m_renderTask; }
    void replaceRenderTask(RenderTask_sV *task);

    float length() const;


    const QDir getDirectory(const QString &name, bool createIfNotExists = true) const;

    QImage interpolateFrameAt(float time, const FrameSize frameSize, float previousTime = -1) const throw(FlowBuildingError);

    FlowField_sV* requestFlow(int leftFrame, int rightFrame, const FrameSize frameSize) const throw(FlowBuildingError);




private:
    QDir m_projDir;
    QString m_projectFilename;

    AbstractFrameSource_sV *m_frameSource;
    AbstractFlowSource_sV *m_flowSource;
    NodeList_sV *m_nodes;
    QList<Tag_sV> *m_tags;
    RenderTask_sV *m_renderTask;

    float timeToFrame(float time) const;

    void init();

};

#endif // PROJECT_SV_H
