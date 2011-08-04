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

QStringList MainWindow::m_commands;

void MainWindow::fillCommandList()
{
    m_commands.clear();
    m_commands << "h:\tHelp";
    m_commands << "q-q:\tQuit";
    m_commands << "n:\tNew";
    m_commands << "o:\tOpen";
    m_commands << "s-s:\tSave";
    m_commands << "s-a:\tSave as";
    m_commands << "x:\tAbort current action";
    m_commands << "x-s:\tAbort selection";
    m_commands << "d-n:\tDelete selected nodes";
    m_commands << "t-s:\tSelect tool";
    m_commands << "t-m:\tMove tool";
    m_commands << "t-t:\tTag";
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_progressDialog(NULL),
    m_renderProgressDialog(NULL),
    m_flowExaminer(NULL)
{
    ui->setupUi(this);

    m_project = new Project_sV();

    m_wCanvas = new Canvas(m_project, this);
    setCentralWidget(m_wCanvas);


    m_wInputMonitor = new FrameMonitor(this);
    m_wInputMonitorDock = new QDockWidget("Input monitor", this);
    m_wInputMonitorDock->setWidget(m_wInputMonitor);
    m_wInputMonitorDock->setObjectName("inputMonitor");
    addDockWidget(Qt::TopDockWidgetArea, m_wInputMonitorDock);

    m_wCurveMonitor = new FrameMonitor(this);
    m_wCurveMonitorDock = new QDockWidget("Curve monitor", this);
    m_wCurveMonitorDock->setWidget(m_wCurveMonitor);
    m_wCurveMonitorDock->setObjectName("curveMonitor");
    addDockWidget(Qt::TopDockWidgetArea, m_wCurveMonitorDock);

    m_wRenderPreview = new RenderPreview(m_project, this);
    m_wRenderPreviewDock = new QDockWidget("Render preview", this);
    m_wRenderPreviewDock->setWidget(m_wRenderPreview);
    m_wRenderPreviewDock->setObjectName("renderPreview");
    addDockWidget(Qt::TopDockWidgetArea, m_wRenderPreviewDock);

    // Fill the view menu that allows (de)activating widgets
    QObjectList windowChildren = children();
    QDockWidget *w;
    for (int i = 0; i < windowChildren.size(); i++) {
        if ((w = dynamic_cast<QDockWidget*>(windowChildren.at(i))) != NULL) {
            qDebug() << "Adding " << w->windowTitle() << " to the menu's widget list";

            QAction *a = new QAction(w->objectName(), this);
            a->setCheckable(true);
            a->setChecked(w->isVisible());
            bool b = true;
            b &= connect(a, SIGNAL(toggled(bool)), w, SLOT(setVisible(bool)));
            b &= connect(w, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            Q_ASSERT(b);

            ui->menuView->addAction(a);
            m_widgetActions << a;

        }
    }


    // Set up shortcut bindings
    m_keyList.insert(MainWindow::Help, "h");
    m_keyList.insert(MainWindow::Quit, "q");
    m_keyList.insert(MainWindow::Quit_Quit, "q");
    m_keyList.insert(MainWindow::New, "n");
    m_keyList.insert(MainWindow::Open, "o");
    m_keyList.insert(MainWindow::Save, "s");
    m_keyList.insert(MainWindow::Save_Same, "s");
    m_keyList.insert(MainWindow::Save_As, "a");
    m_keyList.insert(MainWindow::Abort, "x");
    m_keyList.insert(MainWindow::Abort_Selection, "s");
    m_keyList.insert(MainWindow::Delete, "d");
    m_keyList.insert(MainWindow::Delete_Node, "n");
    m_keyList.insert(MainWindow::Tool, "t");
    m_keyList.insert(MainWindow::Tool_Select, "s");
    m_keyList.insert(MainWindow::Tool_Move, "m");
    m_keyList.insert(MainWindow::Tool_Tag, "t");
    fillCommandList();

    QList<QString> uniqueKeys;
    QList<QString> keys = m_keyList.values();
    for (int i = 0; i < keys.size(); i++) {
        if (!uniqueKeys.contains(keys[i])) {
            uniqueKeys.append(keys[i]);
            qDebug() << "Added to key list: " << keys[i];
        }
    }


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



    bool b = true;

    m_signalMapper = new QSignalMapper(this);
    for (int i = 0; i < uniqueKeys.length(); i++) {

        // Create a new shortcut for each unique key
        QShortcut *shortcut = new QShortcut(QKeySequence(uniqueKeys[i]), this);

        m_shortcutList.append(shortcut);
        m_signalMapper->setMapping(shortcut, uniqueKeys[i]);

        // Connect shortcut to the signal mapper
        b &= connect(shortcut, SIGNAL(activated()), m_signalMapper, SLOT(map()));
    }
    b &= connect(m_signalMapper, SIGNAL(mapped(QString)), this, SLOT(shortcutUsed(QString)));

    b &= connect(this, SIGNAL(deleteNodes()), m_wCanvas, SLOT(slotDeleteNodes()));
    b &= connect(this, SIGNAL(setMode(Canvas::ToolMode)), m_wCanvas, SLOT(slotSetToolMode(Canvas::ToolMode)));
    b &= connect(this, SIGNAL(abort(Canvas::Abort)), m_wCanvas, SLOT(slotAbort(Canvas::Abort)));
    b &= connect(this, SIGNAL(addTag()), m_wCanvas, SLOT(slotAddTag()));

    b &= connect(m_wCanvas, SIGNAL(signalMouseInputTimeChanged(qreal)),
                 this, SLOT(slotForwardInputPosition(qreal)));
    b &= connect(m_wCanvas, SIGNAL(signalMouseCurveSrcTimeChanged(qreal)),
                 this, SLOT(slotForwardCurveSrcPosition(qreal)));

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

    Q_ASSERT(b);

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

    delete m_signalMapper;
    for (int i = 0; i < m_shortcutList.length(); i++) {
        delete m_shortcutList[i];
    }

    for (int i = 0; i < m_widgetActions.size(); i++) {
        delete m_widgetActions[i];
    }
}






////////// Shortcuts

void MainWindow::shortcutUsed(QString which)
{
    TimedShortcut ts;
    ts.shortcut = which;
    ts.start = QTime::currentTime();

    qDebug() << which << " pressed. Last shortcut: " << m_lastShortcut.start.elapsed() << " ms ago.";

//    QString at = QString(" @ %1.%2::%3")
//            .arg(ts.start.minute())
//            .arg(ts.start.second())
//            .arg(ts.start.msec());

    bool handled = false;

    // Use a timeout. Otherwise pressing a key may lead to unpredictable results
    // since it may depend on the key you pressed several minutes ago.
    if (m_lastShortcut.start.elapsed() < 600) {

        // Handle combined shortcuts here.

        if (m_lastShortcut.shortcut == m_keyList[MainWindow::Quit]) {
            if (which == m_keyList[MainWindow::Quit_Quit]) {
                qApp->quit();
            }
        }
        else if (m_lastShortcut.shortcut == m_keyList[MainWindow::Abort]) {
            if (which == m_keyList[MainWindow::Abort_Selection]) {
                emit abort(Canvas::Abort_Selection);
                handled = true;
            }
        }
        else if (m_lastShortcut.shortcut == m_keyList[MainWindow::Delete]) {
            if (which == m_keyList[MainWindow::Delete_Node]) {
                emit deleteNodes();
                handled = true;
            }
        }
        else if (m_lastShortcut.shortcut == m_keyList[MainWindow::Tool]) {
            if (which == m_keyList[MainWindow::Tool_Select]) {
                emit setMode(Canvas::ToolMode_Select);
                handled = true;
            } else if (which == m_keyList[MainWindow::Tool_Move]) {
                emit setMode(Canvas::ToolMode_Move);
                handled = true;
            } else if (which == m_keyList[MainWindow::Tool_Tag]) {
                emit addTag();
                handled = true;
            }
        }
        else if (m_lastShortcut.shortcut == m_keyList[MainWindow::Save]) {
            if (which == m_keyList[MainWindow::Save_Same]) {
                slotSaveProject();
                handled = true;
            } else if (which == m_keyList[MainWindow::Save_As])  {
                slotSaveProjectDialog();
                handled = true;
            }
        }
    } else {
        if (which == m_keyList[MainWindow::Abort]) {
            emit abort(Canvas::Abort_General);
            handled = true;
        } else {
            qDebug() << "(Shortcut timed out.)";
        }
    }
    if (!handled) {
        if (which == m_keyList[MainWindow::Help]) {
            slotToggleHelp();
            handled = true;
        } else if (which == m_keyList[MainWindow::New]) {
            newProject();
            handled = true;
        } else if (which == m_keyList[MainWindow::Open]) {
            slotLoadProjectDialog();
            handled = true;
        }
    }

    m_lastShortcut = ts;
}



////////// Project R/W

void MainWindow::newProject()
{
    NewProjectDialog npd(this);
    if (npd.exec() == QDialog::Accepted) {
        try {
            Project_sV *project = npd.buildProject();

            // Save project
            XmlProjectRW_sV writer;
            writer.saveProject(project, npd.projectFilename());

            loadProject(project);
        } catch (FrameSourceError &err) {
            QMessageBox(QMessageBox::Warning, "Frame source error", err.message()).exec();
        }
    }
}
void MainWindow::loadProject(Project_sV *project)
{
    Q_ASSERT(project != NULL);
    resetDialogs();
    if (m_project != NULL) {
        delete m_project;
        m_project = NULL;
    }
    m_project = project;
    m_wCanvas->load(m_project);
    m_wRenderPreview->load(m_project);

    bool b = true;
    b &= connect(m_project->frameSource(), SIGNAL(signalNextTask(QString,int)), this, SLOT(slotNewFrameSourceTask(QString,int)));
    b &= connect(m_project->frameSource(), SIGNAL(signalAllTasksFinished()), this, SLOT(slotFrameSourceTasksFinished()));
    Q_ASSERT(b);

    m_project->frameSource()->initialize();
}
void MainWindow::slotLoadProjectDialog()
{
    QFileDialog dialog(this, "Load project");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDefaultSuffix("sVproj");
    dialog.setNameFilter("slowmoVideo projects (*.sVproj)");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(m_settings.value("directories/lastOpenedProject", QDir::current().absolutePath()).toString());
    if (dialog.exec() == QDialog::Accepted) {
        m_settings.setValue("directories/lastOpenedProject", QFileInfo(dialog.selectedFiles().at(0)).absolutePath());
        XmlProjectRW_sV reader;
        try {
            QString warning;
            Project_sV *project = reader.loadProject(dialog.selectedFiles().at(0), &warning);
            if (warning.length() > 0) {
                QMessageBox(QMessageBox::Warning, "Warning", warning).exec();
            }
            loadProject(project);
        } catch (FrameSourceError &err) {
            QMessageBox(QMessageBox::Warning, "Frame source error", err.message()).exec();
        } catch (Error_sV &err) {
            QMessageBox(QMessageBox::Warning, "Error", err.message()).exec();
        }
    }
}

void MainWindow::slotSaveProject(QString filename)
{
    if (filename.length() == 0) {
        filename = m_project->projectFilename();
    }
    if (filename.length() == 0) {
        qDebug() << "No filename given, won't save. (Perhaps an empty project?)";
        statusBar()->showMessage("No filename given, won't save. (Perhaps an empty project?)", 5000);
    } else {
        qDebug() << "Saving project as " << filename;
        try {
            XmlProjectRW_sV writer;
            writer.saveProject(m_project, filename);
        } catch (Error_sV &err) {
            QMessageBox(QMessageBox::Warning, "Error writing project file", err.message()).exec();
        }
    }
}
void MainWindow::slotSaveProjectDialog()
{
    QFileDialog dialog(this, "Save project");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("sVproj");
    dialog.setNameFilter("slowmoVideo projects (*.sVproj)");
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
void MainWindow::displayHelp(QPainter &davinci)
{
    QString helpText;
    for (int i = 0; i < m_commands.size(); i++) {
        helpText.append(m_commands.at(i) + ((i < m_commands.size()-1) ? "\n" : ""));
    }

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

        if (m_project->renderTask() != NULL) {
            bool b = true;
            b &= disconnect(SIGNAL(signalRendererContinue()), m_project->renderTask());
            Q_ASSERT(b);
        }
        m_project->replaceRenderTask(task);

        if (m_renderProgressDialog == NULL) {
            m_renderProgressDialog = new ProgressDialog(this);
            m_renderProgressDialog->setWindowTitle("Rendering progress");
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

    }
}
void MainWindow::slotRenderingAborted(QString message)
{
    QMessageBox(QMessageBox::Warning, "Error", message, QMessageBox::Ok).exec();
}

void MainWindow::slotNewFrameSourceTask(const QString taskDescription, int taskSize)
{
    if (m_progressDialog == NULL) {
        m_progressDialog = new ProgressDialog(this);
        m_progressDialog->setWindowTitle("Frame extraction progress");
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
}


