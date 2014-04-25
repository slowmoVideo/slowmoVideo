/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "newProjectDialog.h"
#include "progressDialog.h"
#include "renderingDialog.h"
#include "projectPreferencesDialog.h"
#include "preferencesDialog.h"
#include "aboutDialog.h"

#include "lib/defs_sV.hpp"
#include "project/renderTask_sV.h"
#include "project/xmlProjectRW_sV.h"
#include "project/abstractFrameSource_sV.h"
#include "project/projectPreferences_sV.h"

#include <QtCore>
#include <QObject>
#include <QDockWidget>
#include <QDebug>
#include <QMessageBox>
#include <QtGui/QStatusBar>

#include <QDir>
#include <QFileDialog>

#include <QShortcut>
#include <QSignalMapper>
#include <QTime>
#include <QFuture>

#include <QPainter>

#include <functional>

MainWindow::MainWindow(QString projectPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_progressDialog(NULL),
    m_renderProgressDialog(NULL),
    m_flowExaminer(NULL),
    m_cs(this)
{
    ui->setupUi(this);

    restoreGeometry(m_settings.value("geometry").toByteArray());
    restoreState(m_settings.value("windowState").toByteArray());

    m_project = new Project_sV();

    m_wCanvas = new Canvas(m_project, this);
    setCentralWidget(m_wCanvas);


    m_wInputMonitor = new FrameMonitor(this);
    m_wInputMonitorDock = new QDockWidget(tr("Input monitor"), this);
    m_wInputMonitorDock->setWidget(m_wInputMonitor);
    m_wInputMonitorDock->setObjectName("inputMonitor");
    addDockWidget(Qt::TopDockWidgetArea, m_wInputMonitorDock);

    m_wCurveMonitor = new FrameMonitor(this);
    m_wCurveMonitorDock = new QDockWidget(tr("Curve monitor"), this);
    m_wCurveMonitorDock->setWidget(m_wCurveMonitor);
    m_wCurveMonitorDock->setObjectName("curveMonitor");
    addDockWidget(Qt::TopDockWidgetArea, m_wCurveMonitorDock);

    m_wRenderPreview = new RenderPreview(m_project, this);
    m_wRenderPreviewDock = new QDockWidget(tr("Render preview"), this);
    m_wRenderPreviewDock->setWidget(m_wRenderPreview);
    m_wRenderPreviewDock->setObjectName("renderPreview");
    addDockWidget(Qt::TopDockWidgetArea, m_wRenderPreviewDock);

    // Fill the view menu that allows (de)activating widgets
    QObjectList windowChildren = children();
    QDockWidget *w;
    for (int i = 0; i < windowChildren.size(); i++) {
        if ((w = dynamic_cast<QDockWidget*>(windowChildren.at(i))) != NULL) {
            qDebug() << "Adding " << w->windowTitle() << " to the menu's widget list";

            QAction *a = new QAction("&" + w->objectName(), this);
            a->setCheckable(true);
            bool b = true;
            b &= connect(a, SIGNAL(toggled(bool)), w, SLOT(setVisible(bool)));
            // This does not work since it is also emitted e.g. when the window is minimized
            // (with «Show Desktop» on KDE4), therefore an event filter is required. (below.)
            // Thanks ArGGu^^ for the tip!
//            b &= connect(w, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            Q_ASSERT(b);
            a->setChecked(true);

            // To uncheck the menu entry when the widget is closed via the (x)
            w->installEventFilter(this);

            ui->menuView->addAction(a);
            m_widgetActions << a;

        }
    }




    ui->actionNew->setShortcut(QKeySequence("Ctrl+N"));
    ui->actionOpen->setShortcut(QKeySequence("Ctrl+O"));
    ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionSave_as->setShortcut(QKeySequence("Shift+Ctrl+S"));
    ui->actionShortcuts->setShortcut(QKeySequence("Ctrl+H"));
    ui->actionRender->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionRenderPreview->setShortcut(QKeySequence("Shift+Ctrl+R"));
    ui->actionExamineFlow->setShortcut(QKeySequence("Shift+Ctrl+X"));
    ui->actionPreferences->setShortcut(QKeySequence("Ctrl+,"));
    ui->actionAbout->setShortcut(QKeySequence("F1"));
    ui->actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
    ui->actionZoomIn->setShortcut(QKeySequence("+"));
    ui->actionZoomOut->setShortcut(QKeySequence("-"));

    m_cs.addShortcut("h", Help, tr("Show help overlay"));
    m_cs.addShortcut("q-q", Quit, tr("Quit"));
    m_cs.addShortcut("n", New, tr("New project"));
    m_cs.addShortcut("o", Open, tr("Open project"));
    m_cs.addShortcut("s-s", Save_Same, tr("Save"));
    m_cs.addShortcut("s-a", Save_As, tr("Save as ..."));
    m_cs.addShortcut("a", Abort, tr("Abort move"));
    m_cs.addShortcut("a-s", Abort_Selection, tr("Unselect all"));
    m_cs.addShortcut("d-n", Delete_Node, tr("Delete selected nodes"));
    m_cs.addShortcut("t-s", Tool_Select, tr("Selecting tool"));
    m_cs.addShortcut("t-m", Tool_Move, tr("Move tool"));
    m_cs.addShortcut("t-t", Tag, tr("Insert label (tag)"));

    bool b = true;
    b &= connect(&m_cs, SIGNAL(signalShortcutUsed(int)), this, SLOT(slotShortcutUsed(int)));

    b &= connect(this, SIGNAL(deleteNodes()), m_wCanvas, SLOT(slotDeleteNodes()));
    b &= connect(this, SIGNAL(setMode(Canvas::ToolMode)), m_wCanvas, SLOT(slotSetToolMode(Canvas::ToolMode)));
    b &= connect(this, SIGNAL(abort(Canvas::Abort)), m_wCanvas, SLOT(slotAbort(Canvas::Abort)));
    b &= connect(this, SIGNAL(addTag()), m_wCanvas, SLOT(slotAddTag()));

    b &= connect(ui->actionZoomIn, SIGNAL(triggered()), m_wCanvas, SLOT(slotZoomIn()));
    b &= connect(ui->actionZoomOut, SIGNAL(triggered()), m_wCanvas, SLOT(slotZoomOut()));

    b &= connect(m_wCanvas, SIGNAL(signalMouseInputTimeChanged(qreal)),
                 this, SLOT(slotForwardInputPosition(qreal)));
    b &= connect(m_wCanvas, SIGNAL(signalMouseCurveSrcTimeChanged(qreal)),
                 this, SLOT(slotForwardCurveSrcPosition(qreal)));

    b &= connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(slotNewProject()));
    b &= connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(slotLoadProjectDialog()));
    b &= connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(slotSaveProject()));
    b &= connect(ui->actionSave_as, SIGNAL(triggered()), this, SLOT(slotSaveProjectDialog()));
    b &= connect(ui->actionRender, SIGNAL(triggered()), this, SLOT(slotShowRenderDialog()));
    b &= connect(ui->actionRenderPreview, SIGNAL(triggered()), this, SLOT(slotUpdateRenderPreview()));
    b &= connect(ui->actionExamineFlow, SIGNAL(triggered()), this, SLOT(slotShowFlowExaminerDialog()));
    b &= connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(slotShowPreferencesDialog()));
    b &= connect(ui->actionShortcuts, SIGNAL(triggered()), this, SLOT(slotToggleHelp()));
    b &= connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(slotShowAboutDialog()));
    b &= connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    b &= connect(ui->actionProjectPreferences, SIGNAL(triggered()), this, SLOT(slotShowProjectPreferencesDialog()));

    Q_ASSERT(b);

    updateWindowTitle();
    setWindowIcon(QIcon(":icons/slowmoIcon.png"));

    QSettings settings;
    bool show = settings.value("ui/displayHelp", true).toBool();
    m_wCanvas->showHelp(show);
    settings.sync();

   
    if (!projectPath.isEmpty()) {
        loadProject(projectPath);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_wInputMonitor;
    delete m_wCurveMonitor;
    delete m_wInputMonitorDock;
    delete m_wCurveMonitorDock;
    delete m_wRenderPreview;
    delete m_wRenderPreviewDock;

    if (m_project != NULL) {
        delete m_project;
    }

    if (m_progressDialog != NULL) {
        delete m_progressDialog;
    }
    if (m_renderProgressDialog != NULL) {
        delete m_renderProgressDialog;
    }
    if (m_flowExaminer != NULL) {
        delete m_flowExaminer;
    }

    for (int i = 0; i < m_widgetActions.size(); i++) {
        delete m_widgetActions[i];
    }
}

bool MainWindow::okToContinue()
{
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("slowmoUI"),
                        tr("The document has been modified.\n"
                           "Do you want to save your changes?"),
                        QMessageBox::Yes | QMessageBox::No
                        | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            slotSaveProjectDialog();
            return true;
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	if (okToContinue()) {
	    m_settings.setValue("geometry", saveGeometry());
    	m_settings.setValue("windowState", saveState());
	    QMainWindow::closeEvent(e);
	}
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    QObjectList windowChildren = children();
    QDockWidget *w;

    if (e->type() == QEvent::Close && windowChildren.contains(obj)) {
        if ((w = dynamic_cast<QDockWidget *>(obj)) != NULL) {

            QList<QAction*> actions = findChildren<QAction *>();
            for (int i = 0; i < actions.size(); i++) {
                if (actions.at(i)->text() == w->objectName()) {
                    actions.at(i)->setChecked(false);
                    return true;
                }
            }

        }
    }

    return false;
}






////////// Shortcuts

void MainWindow::slotShortcutUsed(int id)
{
    if (id == Quit) {
        qApp->quit();
    } else if (id == Abort_Selection) {
        emit abort(Canvas::Abort_Selection);
    } else if (id == Delete_Node) {
        emit deleteNodes();
    } else if (id == Tool_Select) {
        emit setMode(Canvas::ToolMode_Select);
    } else if (id == Tool_Move) {
        emit setMode(Canvas::ToolMode_Move);
    } else if (id == Tag) {
        emit addTag();
    } else if (id == Save_Same) {
        slotSaveProject();
    } else if (id == Save_As) {
        slotSaveProjectDialog();
    } else if (id == Abort) {
        emit abort(Canvas::Abort_General);
    } else if (id == Help) {
        slotToggleHelp();
    } else if (id == New) {
        slotNewProject();
    } else if (id == Open) {
        slotLoadProjectDialog();
    }
}


////////// Project R/W

void MainWindow::slotNewProject()
{
    NewProjectDialog npd(this);
    if (npd.exec() == QDialog::Accepted) {
        try {
            Project_sV *project = npd.buildProject();

            // Save project
            XmlProjectRW_sV writer;
            writer.saveProject(project, npd.projectFilename());
            m_projectPath = npd.projectFilename();

            project->preferences()->viewport_secRes() = QPointF(400, 400)/project->frameSource()->framesCount()*project->frameSource()->fps()->fps();
            loadProject(project);

            m_wCanvas->showHelp(true);
			setWindowModified(true);

        } catch (FrameSourceError &err) {
            QMessageBox(QMessageBox::Warning, "Frame source error", err.message()).exec();
        }
    }
}

void MainWindow::loadProject(Project_sV *project)
{
    Q_ASSERT(project != NULL);
    resetDialogs();

    Project_sV *projTemp = NULL;
    if (m_project != NULL) {
        projTemp = m_project;
    }
    m_project = project;
    m_wCanvas->load(m_project);
    m_wRenderPreview->load(m_project);
    updateWindowTitle();

    if (projTemp != NULL) {
        // Do not delete the old project object earlier to avoid segfaults
        // (may still be used in the ShutterFunction dialog e.g.)
        delete projTemp;
    }

    bool b = true;
    b &= connect(m_project->frameSource(), SIGNAL(signalNextTask(QString,int)), this, SLOT(slotNewFrameSourceTask(QString,int)));
    b &= connect(m_project->frameSource(), SIGNAL(signalAllTasksFinished()), this, SLOT(slotFrameSourceTasksFinished()));
    Q_ASSERT(b);

    m_project->frameSource()->initialize();
    
}
void MainWindow::slotLoadProjectDialog()
{
	if (okToContinue()) {
	    QString dir = m_settings.value("directories/lastOpenedProject", QDir::current().absolutePath()).toString();
    	QString file = QFileDialog::getOpenFileName(this, tr("Load Project"), dir, tr("slowmoVideo projects (*.sVproj)"));

	    if (!file.isEmpty()) {
    	    qDebug() << file;
        	loadProject(QFileInfo(file).absoluteFilePath());
	    }
	}
}

void MainWindow::loadProject(QString path)
{
    m_settings.setValue("directories/lastOpenedProject", path);
    XmlProjectRW_sV reader;
    try {
        QString warning;
        Project_sV *project = reader.loadProject(path, &warning);
        if (warning.length() > 0) {
            QMessageBox(QMessageBox::Warning, tr("Warning"), warning).exec();
        }
        m_projectPath = path;
        loadProject(project);
    } catch (FrameSourceError &err) {
        QMessageBox(QMessageBox::Warning, tr("Frame source error"), err.message()).exec();
    } catch (Error_sV &err) {
        QMessageBox(QMessageBox::Warning, tr("Error"), err.message()).exec();
    }
}


void MainWindow::slotSaveProject(QString filename)
{
    if (filename.length() == 0) {
        filename = m_project->projectFilename();
    }
    if (filename.length() == 0) {
        qDebug() << "No filename given, won't save. (Perhaps an empty project?)";
        statusBar()->showMessage(tr("No filename given, won't save. (Perhaps an empty project?)"), 5000);
    } else {
        qDebug() << "Saving project as " << filename;
        try {
            XmlProjectRW_sV writer;
            writer.saveProject(m_project, filename);
            statusBar()->showMessage(QString(tr("Saved project as: %1")).arg(filename));
        } catch (Error_sV &err) {
            QMessageBox(QMessageBox::Warning, tr("Error writing project file"), err.message()).exec();
        }
    }
}
void MainWindow::slotSaveProjectDialog()
{
    QFileDialog dialog(this, tr("Save project"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("sVproj");
    dialog.setNameFilter(tr("slowmoVideo projects (*.sVproj)"));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(QFileInfo(m_project->projectFilename()).absolutePath());
    if (dialog.exec() == QDialog::Accepted) {
        slotSaveProject(dialog.selectedFiles().at(0));
    }
}



////////// UI interaction

void MainWindow::slotToggleHelp()
{
    m_wCanvas->toggleHelp();
}
void MainWindow::displayHelp(QPainter &davinci) const
{
    QString helpText = m_cs.shortcutList()
            + tr("\nNavigation: [Shift] Scroll, Drag")
            + tr("\nMove nodes: [Ctrl]  Drag");

    QRect content;
    const QPoint topLeft(10, 10);
    const QPoint padding(10, 10);

    // Check how big the text's bounding box will be
    davinci.drawText(QRect(0,0,0,0), Qt::AlignLeft | Qt::TextExpandTabs, helpText, &content);

    // Draw the background
    content.adjust(topLeft.x(), topLeft.y(), topLeft.x()+2*padding.x(), topLeft.y()+2*padding.y());
    davinci.fillRect(content, QColor(0,0,40, 200));

    // Really draw the text now
    content.translate(padding);
    davinci.drawText(content, Qt::AlignLeft, helpText, &content);
}
void MainWindow::slotForwardInputPosition(qreal frame)
{
    if (0 <= frame && frame < m_project->frameSource()->framesCount()) {
        m_wInputMonitor->slotLoadImage(m_project->frameSource()->framePath(qFloor(frame), FrameSize_Small));
    }
}
void MainWindow::slotForwardCurveSrcPosition(qreal frame)
{
    if (0 <= frame && frame < m_project->frameSource()->framesCount()) {
        m_wCurveMonitor->slotLoadImage(m_project->frameSource()->framePath(qFloor(frame), FrameSize_Orig));
    }
}
void MainWindow::slotUpdateRenderPreview()
{
    m_wRenderPreview->slotRenderAt(m_project->snapToOutFrame(
                                       m_wCanvas->prevMouseTime().x(), false,
                                       m_project->preferences()->renderFPS(), NULL)
                                   );
}


void MainWindow::updateWindowTitle()
{
    QString project(tr("empty project"));
    if (m_projectPath.length() > 0) {
        project = m_projectPath;        
    }
    setWindowTitle(QString("slowmo UI (%1) [*]").arg(project));
}



////////// Dialogues

void MainWindow::resetDialogs()
{
    if (m_progressDialog != NULL) {
        m_progressDialog->close();
        delete m_progressDialog;
        m_progressDialog = NULL;
    }
    if (m_renderProgressDialog != NULL) {
        m_renderProgressDialog->close();
        delete m_renderProgressDialog;
        m_renderProgressDialog = NULL;
    }
    if (m_flowExaminer != NULL) {
        m_flowExaminer->close();
        delete m_flowExaminer;
        m_flowExaminer = NULL;
    }
}

void MainWindow::slotShowAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::slotShowPreferencesDialog()
{
    PreferencesDialog dialog;
    dialog.exec();

    // Use the new flow method (if it has changed)
    m_project->reloadFlowSource();
}

void MainWindow::slotShowProjectPreferencesDialog()
{
    ProjectPreferencesDialog ppd(m_project->preferences(), this);
    ppd.exec();
}

void MainWindow::slotShowFlowExaminerDialog()
{
    if (m_flowExaminer == NULL) {
        m_flowExaminer = new FlowExaminer(m_project, this);
    }

    int frame = floor(m_wCanvas->prevMouseInFrame());
    if (frame+1 >= m_project->frameSource()->framesCount()) {
        frame = m_project->frameSource()->framesCount()-2;
    }
    if (frame < 0) { frame = 0; }

    m_flowExaminer->show();
    m_flowExaminer->examine(frame);
}

void MainWindow::slotShowRenderDialog()
{
    RenderingDialog renderingDialog(m_project, this);
    if (renderingDialog.exec() == QDialog::Accepted) {

        RenderTask_sV *task = renderingDialog.buildTask();
        task->moveToThread(&m_rendererThread);

        if (m_project->renderTask() != NULL) {
            bool b = true;
            b &= disconnect(SIGNAL(signalRendererContinue()), m_project->renderTask());
            Q_ASSERT(b);
        }
        m_project->replaceRenderTask(task);


        if (m_renderProgressDialog == NULL) {
            m_renderProgressDialog = new ProgressDialog(this);
            m_renderProgressDialog->setWindowTitle(tr("Rendering progress"));
        } else {
            m_renderProgressDialog->disconnect();
        }

        bool b = true;
        b &= connect(task, SIGNAL(signalNewTask(QString,int)), m_renderProgressDialog, SLOT(slotNextTask(QString,int)));
        b &= connect(task, SIGNAL(signalItemDesc(QString)), m_renderProgressDialog, SLOT(slotTaskItemDescription(QString)));
        b &= connect(task, SIGNAL(signalTaskProgress(int)), m_renderProgressDialog, SLOT(slotTaskProgress(int)));
        b &= connect(task, SIGNAL(signalRenderingFinished(QString)), m_renderProgressDialog, SLOT(slotAllTasksFinished(QString)));
        b &= connect(task, SIGNAL(signalRenderingAborted(QString)), this, SLOT(slotRenderingAborted(QString)));
        b &= connect(task, SIGNAL(signalRenderingAborted(QString)), m_renderProgressDialog, SLOT(close()));
        b &= connect(task, SIGNAL(signalRenderingStopped(QString)), m_renderProgressDialog, SLOT(slotAborted(QString)));
        b &= connect(m_renderProgressDialog, SIGNAL(signalAbortTask()), task, SLOT(slotStopRendering()));
        b &= connect(this, SIGNAL(signalRendererContinue()), task, SLOT(slotContinueRendering()), Qt::UniqueConnection);
        // TODO continue/abort
        Q_ASSERT(b);

        m_renderProgressDialog->show();

        emit signalRendererContinue();
        m_rendererThread.start();

    }
}
void MainWindow::slotRenderingAborted(QString message)
{
    QMessageBox(QMessageBox::Warning, tr("Error"), message, QMessageBox::Ok).exec();
}

void MainWindow::slotNewFrameSourceTask(const QString taskDescription, int taskSize)
{
    if (m_progressDialog == NULL) {
        m_progressDialog = new ProgressDialog(this);
        m_progressDialog->setWindowTitle(tr("Frame extraction progress"));
        bool b = true;
        b &= connect(m_project->frameSource(), SIGNAL(signalNextTask(QString,int)), m_progressDialog, SLOT(slotNextTask(QString,int)));
        b &= connect(m_project->frameSource(), SIGNAL(signalTaskProgress(int)), m_progressDialog, SLOT(slotTaskProgress(int)));
        b &= connect(m_project->frameSource(), SIGNAL(signalTaskItemDescription(QString)), m_progressDialog, SLOT(slotTaskItemDescription(QString)));
        b &= connect(m_project->frameSource(), SIGNAL(signalAllTasksFinished()), m_progressDialog, SLOT(slotAllTasksFinished()));
        b &= connect(m_progressDialog, SIGNAL(signalAbortTask()), m_project->frameSource(), SLOT(slotAbortInitialization()));
        Q_ASSERT(b);
    }
    m_progressDialog->show();
    m_progressDialog->slotNextTask(taskDescription, taskSize);
}
void MainWindow::slotFrameSourceTasksFinished()
{
    QTimer::singleShot(200, this, SLOT(slotCloseFrameSourceProgress()));
}
void MainWindow::slotCloseFrameSourceProgress()
{
    if (m_progressDialog != NULL) {
        m_progressDialog->close();
    }
    //is right place ? should we check ?
    m_project->buildCacheFlowSource();
}


