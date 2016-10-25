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
#include "renderPreferences_sV.h"
#include "../lib/defs_sV.hpp"
#include "../lib/videoInfo_sV.h"


#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include <QThread>

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
class WorkerFlow;

/**
  \brief slowmoVideo project holding all important information.
  \todo Check if width%4 == 0 for V3D
  */
class Project_sV : public QObject
{
    Q_OBJECT

public:
    /** Creates an empty project. A video file can be loaded with loadFile(QString, QString). */
    Project_sV();
    /**
      Creates a new project.
      \param filename Input video file
      \param projectDir Project directory; All cached files will be put in there.
      */
    Project_sV(QString projectDir);
    ~Project_sV();

    ProjectPreferences_sV* preferences() { return m_preferences; }

    void setProjectDir(QString projectDir);
    void setupProjectDir(); // TODO:
    QDir getProjectDir() { return m_projDir; };
    /** The project filename should be set when saving or loading the project. */
    void setProjectFilename(QString filename);
    /** \return The filename this project was last saved as. */
    QString projectFilename() const;

    /** \param frameSource will be managed (deleted) by the project. If \c NULL, an empty frame source will be used. */
    void loadFrameSource(AbstractFrameSource_sV *frameSource);

    AbstractFrameSource_sV* frameSource() { return m_frameSource; }
    AbstractFlowSource_sV* flowSource() { return m_flowSource; }
    NodeList_sV *nodes() const { return m_nodes; }
    QList<Tag_sV> *tags() const { return m_tags; }
    ShutterFunctionList_sV* shutterFunctions() { return m_shutterFunctions; }
    MotionBlur_sV *motionBlur() { return m_motionBlur; }

    /** \see replaceRenderTask() */
    RenderTask_sV *renderTask() { return m_renderTask; }
    void replaceRenderTask(RenderTask_sV *task);


    /**
      \fn snapToFrame()
      \brief Snaps in the given time on a grid given by the number of frames per second.
      This allows to, for example, render from 0 to 3.2 seconds and then from 3.2 to 5 seconds
      to images with the same effect as rendering all at once. Always starts from 0!
      \param time Time to snap in
      \param roundUp To chose between rounding up or down
      \param fps Frames per second to use.
      */
    /**
      \fn toOutTime()
      \brief Converts a time expression like \c f:123 or \c :start to an output time \c float.

      Accepted input format:
      \li \c 24.3 or \c t:24.3 for 24.3 seconds
      \li \c f:123 for frame 123
      \li \c p:25% for 25 %
      \li \c l:slowdown for the slowdown label (tag)
      \li \c :start and \c :end for project start/end
      */
    static qreal snapToFrame(const qreal time, bool roundUp, const Fps_sV& fps, int* out_framesBeforeHere);
    qreal snapToOutFrame(qreal time, bool roundUp, const Fps_sV& fps, int* out_framesBeforeHere) const;
    qreal toOutTime(QString timeExpression, const Fps_sV& fps) const throw(Error_sV);


    const QDir getDirectory(const QString &name, bool createIfNotExists = true) const;

    QImage render(qreal outTime, RenderPreferences_sV prefs);

    FlowField_sV* requestFlow(int leftFrame, int rightFrame, const FrameSize frameSize) throw(FlowBuildingError);

    /**
      \brief Searches for objects near the given \c pos.
      This search includes tags.
      \see NodeList_sV::objectsNear() Used by this method. Does not include tags.
      */
    QList<NodeList_sV::PointerWithDistance> objectsNear(QPointF pos, qreal tmaxdist) const;


public:
    /// Reload the flow source in case the user changed the default (preferred) method.
    void reloadFlowSource();

    // prebuilt the need optical flow files
    void buildCacheFlowSource();


private:
    QDir m_projDir;
    QString m_projectFilename;

    ProjectPreferences_sV *m_preferences;

    AbstractFrameSource_sV *m_frameSource;
    AbstractFlowSource_sV *m_flowSource;
    MotionBlur_sV *m_motionBlur;

    NodeList_sV *m_nodes;
    QList<Tag_sV> *m_tags;
    //TODO: remove this
    RenderTask_sV *m_renderTask;
    ShutterFunctionList_sV *m_shutterFunctions;

    qreal sourceTimeToFrame(qreal time) const;

    QDir getDirectoryName();
    void init();

    /**
     * @brief Thread object which will let us manipulate the running thread
     */
    QThread *thread[4];
    /**
     * @brief Object which contains methods that should be runned in another thread
     */
    WorkerFlow *worker[4];

    
private:
    /// Count how many times V3D failed, after a certain limit we assume the user does not have an nVidia card
    /// and constantly switch to OpenCV
    int m_v3dFailCounter;

    // launch a worker thread for optical flow
    void startFlow(int threadid,const FrameSize frameSize,int direction);
};

#endif // PROJECT_SV_H
