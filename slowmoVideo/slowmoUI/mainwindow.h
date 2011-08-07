/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canvas.h"
#include "frameMonitor.h"
#include "renderPreview.h"
#include "dialogues/flowExaminer.h"
#include "project/project_sV.h"

namespace Ui {
    class MainWindow;
}

#include <QtGui/QMainWindow>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QTime>
#include <QtCore/QThread>

class Canvas;
class ProgressDialog;
class QShortcut;
class QSignalMapper;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct TimedShortcut {
        QTime start;
        QString shortcut;
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum ShortcutCommands {
        Quit,
        Quit_Quit,
        Abort,
        Abort_Selection,
        Delete,
        Delete_Node,
        Tool,
        Tool_Select,
        Tool_Move,
        Tool_Tag,
        Help,
        New,
        Open,
        Save,
        Save_Same,
        Save_As
    };

    static void displayHelp(QPainter &davinci);
    static QStringList m_commands;

protected slots:
    virtual void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow *ui;
    QSettings m_settings;

    Project_sV *m_project;
    QString m_projectPath;

    Canvas *m_wCanvas;
    FrameMonitor *m_wInputMonitor;
    FrameMonitor *m_wCurveMonitor;
    QDockWidget *m_wInputMonitorDock;
    QDockWidget *m_wCurveMonitorDock;
    RenderPreview* m_wRenderPreview;
    QDockWidget *m_wRenderPreviewDock;
    QList<QAction*> m_widgetActions;

    ProgressDialog *m_progressDialog;
    ProgressDialog *m_renderProgressDialog;
    FlowExaminer *m_flowExaminer;


    TimedShortcut m_lastShortcut;

    QSignalMapper *m_signalMapper;
    QList<QShortcut *> m_shortcutList;
    QMap<int, QString> m_keyList;

    QThread m_rendererThread;



    static void fillCommandList();

    void newProject();
    void loadProject(Project_sV *project);
    void resetDialogs();
    void updateWindowTitle();

private slots:
    /**
      Handles timed shortcuts.
      Example: Pressing q twice quits the application iff the elapsed time
      between the two key strokes is small enough.
     */
    void shortcutUsed(QString);
    void slotShowRenderDialog();
    void slotShowPreferencesDialog();
    void slotShowFlowExaminerDialog();
    void slotForwardInputPosition(qreal frame);
    void slotForwardCurveSrcPosition(qreal frame);

    void slotNewFrameSourceTask(const QString taskDescription, int taskSize);
    void slotFrameSourceTasksFinished();
    void slotCloseFrameSourceProgress();

    void slotRenderingAborted(QString message);

    void slotSaveProject(QString filename = "");
    void slotSaveProjectDialog();
    void slotLoadProjectDialog();

    void slotToggleHelp();
    void slotShowAboutDialog();
    void slotUpdateRenderPreview();


signals:
    void deleteNodes();
    void setMode(const Canvas::ToolMode mode);
    void abort(const Canvas::Abort abort);
    void addTag();
    void signalRendererContinue();

};

#endif // MAINWINDOW_H
