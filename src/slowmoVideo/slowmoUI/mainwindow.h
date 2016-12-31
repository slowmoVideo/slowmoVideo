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
#include "../project/project_sV.h"
#include "../libgui/combinedShortcuts.h"

namespace Ui {
    class MainWindow;
}

#include <QMainWindow>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QTime>
#include <QtCore/QThread>
#include <QSettings>

class Canvas;
class ProgressDialog;
class QShortcut;
class QSignalMapper;

namespace Ui {
    class MainWindow;
}

/// \todo Call flow editor from here
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString projectPath = QString(), QWidget *parent = 0);
    ~MainWindow();

    void displayHelp(QPainter &davinci) const;

protected slots:
    virtual void closeEvent(QCloseEvent *e);
#if QT_VERSION <= QT_VERSION_CHECK(4, 2, 0)
    virtual bool eventFilter(QObject *obj, QEvent *e);
#endif

private:
    enum ShortcutCommands {
        Quit,
        Abort,
        Abort_Selection,
        Delete_Node,
        Tool_Select,
        Tool_Move,
        Tag,
        Help,
        New,
        Open,
        Save_Same,
        Save_As
    };

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


    CombinedShortcuts m_cs;

    QThread m_rendererThread;

    void createDockWindows();
    void createActions();
    
	void loadProject(QString path);
    void loadProject(Project_sV *project);
    void resetDialogs();
    void updateWindowTitle();
	bool okToContinue();
	
	private slots:
    void slotShortcutUsed(int id);
    void slotShowRenderDialog();
    void slotShowPreferencesDialog();
    void slotShowProjectPreferencesDialog();
    void slotShowFlowExaminerDialog();
    void slotShowDebugWindow(bool set);
    void slotForwardInputPosition(qreal frame);
    void slotForwardCurveSrcPosition(qreal frame);

    void slotNewFrameSourceTask(const QString taskDescription, int taskSize);
    void slotFrameSourceTasksFinished();
    void slotCloseFrameSourceProgress();

    void slotRenderingAborted(QString message);

    void slotNewProject();
    void slotSaveProject(QString filename = "");
    void slotSaveProjectDialog();
    void slotLoadProjectDialog();

    void slotToggleHelp();
    void slotShowAboutDialog();
    void slotUpdateRenderPreview();

	void slotShowFlowEditWindow();
	
signals:
    void deleteNodes();
    void setMode(const Canvas::ToolMode mode);
    void abort(const Canvas::Abort abort);
    void addTag();
    void signalRendererContinue();

};

#endif // MAINWINDOW_H
