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
#include "nodeList_sV.h"
#include "../lib/defs_sV.hpp"
extern "C" {
#include "../lib/videoInfo_sV.h"
}

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include <QSettings>

class ProjectPreferences_sV;
class Flow_sV;
class AbstractFrameSource_sV;
class AbstractFlowSource_sV;
class MotionBlur_sV;
class ShutterFunctionList_sV;
class RenderTask_sV;
class FlowField_sV;
class QSignalMapper;
class QProcess;
class QRegExp;
class QTimer;

/** \brief slowmoVideo project holding all important information. */
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

    ProjectPreferences_sV* preferences() { return m_preferences; }

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
    ShutterFunctionList_sV* shutterFunctions() { return m_shutterFunctions; }
    MotionBlur_sV *motionBlur() { return m_motionBlur; }

    RenderTask_sV *renderTask() { return m_renderTask; }
    void replaceRenderTask(RenderTask_sV *task);


    /**
      \brief Snaps in the given time on a grid given by the number of frames per second.
      This allows to, for example, render from 0 to 3.2 seconds and then from 3.2 to 5 seconds
      to images with the same effect as rendering all at once. Always starts from 0!
      \param time Time to snap in
      \param roundUp To chose between rounding up or down
      \param fps Frames per second to use.
      */
    static float snapToFrame(const float time, bool roundUp, const Fps_sV& fps, int* out_framesBeforeHere);
    float snapToOutFrame(float time, bool roundUp, const Fps_sV& fps, int* out_framesBeforeHere) const;


    const QDir getDirectory(const QString &name, bool createIfNotExists = true) const;

    QImage render(float outTime, Fps_sV fps, InterpolationType interpolation, FrameSize size);

    FlowField_sV* requestFlow(int leftFrame, int rightFrame, const FrameSize frameSize) const throw(FlowBuildingError);

    /**
      \brief Searches for objects near the given \c pos.
      This search includes tags.
      \see NodeList_sV::objectsNear() Used by this method. Does not include tags.
      */
    QList<NodeList_sV::PointerWithDistance> objectsNear(QPointF pos, qreal tmaxdist) const;



private:
    QDir m_projDir;
    QString m_projectFilename;

    ProjectPreferences_sV *m_preferences;

    AbstractFrameSource_sV *m_frameSource;
    AbstractFlowSource_sV *m_flowSource;
    MotionBlur_sV *m_motionBlur;

    NodeList_sV *m_nodes;
    QList<Tag_sV> *m_tags;
    RenderTask_sV *m_renderTask;
    ShutterFunctionList_sV *m_shutterFunctions;

    float sourceTimeToFrame(float time) const;

    void init();

};

#endif // PROJECT_SV_H
