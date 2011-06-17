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

#include <QFile>
#include <QDir>
#include <QString>
#include <QImage>

#include <QObject>
#include <QFile>

class Flow_sV;
class AbstractFrameSource_sV;
#include "../lib/defs_sV.hpp"

extern "C" {
#include "../lib/videoInfo_sV.h"
}

#include "tag_sV.h"

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


    /** \param frameSource will be managed (deleted) by the project. If \c NULL, an empty frame source will be used. */
    void loadFrameSource(AbstractFrameSource_sV *frameSource);

    /** Load filename and set the project directory to projectDir */
    void loadFile(QString filename, QString projectDir);


    AbstractFrameSource_sV* frameSource() { return m_frameSource; }
    Flow_sV *flow() const { return m_flow; }
    NodeList_sV *nodes() const { return m_nodes; }
    QList<Tag_sV> *tags() const { return m_tags; }
    RenderTask_sV *renderTask() { return m_renderTask; }
    const FrameSize renderFrameSize() const { return m_renderFrameSize; }
    float fpsOut() const { return m_fps; }
    float length() const;


    void render(qreal fps);


    /**
      @return true, iff all required directories exist
      */
    bool validDirectories() const;

    QImage interpolateFrameAt(float time, const FrameSize frameSize) const;

    FlowField_sV* requestFlow(int leftFrame, FlowDirection direction, const FrameSize frameSize, bool forceRebuild = false) const;

    const QDir getDirectory(const QString &name, bool createIfNotExists = true) const;

    const QString frameFileStr(int number, FrameSize size) const;
    const QString flowFileStr(int leftFrame, FlowDirection direction, FrameSize size) const;
    const QString renderedFileStr(int number, FrameSize size) const;

public slots:
    void slotSetFps(float fps);
    void slotSetRenderFrameSize(const FrameSize size);


private:
    QDir m_projDir;
    AbstractFrameSource_sV *m_frameSource;
    Flow_sV *m_flow;
    NodeList_sV *m_nodes;
    QList<Tag_sV> *m_tags;
    RenderTask_sV *m_renderTask;

    float m_fps;
    FrameSize m_renderFrameSize;

    float timeToFrame(float time) const;

    void init();
    void createDirectories(FrameSize frameSize) const;
    const QString framesDirStr(FrameSize frameSize) const; // TODO remove?
    const QString flowDirStr(FrameSize frameSize) const;
    const QString renderDirStr(FrameSize frameSize) const;

};

#endif // PROJECT_SV_H
