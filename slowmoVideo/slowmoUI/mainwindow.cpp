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

#include <QDebug>

#include <QShortcut>
#include <QSignalMapper>
#include <QTime>

#include <QPainter>

QStringList MainWindow::m_commands;

void MainWindow::fillCommandList()
{
    m_commands.clear();
    m_commands << "h:\tHelp";
    m_commands << "q-q:\tQuit";
    m_commands << "x:\tAbort current action";
    m_commands << "x-s:\tAbort selection";
    m_commands << "d-n:\tDelete selected nodes";
    m_commands << "t-s:\tSelect tool";
    m_commands << "t-a:\tAdd tool";
    m_commands << "t-m:\tMove tool";
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_project("/data/Videos/2010-09-14-DSC_5111.AVI", "/tmp")
{
    ui->setupUi(this);


    m_wCanvas = new Canvas(this);
    setCentralWidget(m_wCanvas);
    m_wCanvas->load(m_project);


    // Set up shortcut bindings
    m_keyList.insert(MainWindow::Help, "h");
    m_keyList.insert(MainWindow::Quit, "q");
    m_keyList.insert(MainWindow::Quit_Quit, "q");
    m_keyList.insert(MainWindow::Abort, "x");
    m_keyList.insert(MainWindow::Abort_Selection, "s");
    m_keyList.insert(MainWindow::Delete, "d");
    m_keyList.insert(MainWindow::Delete_Node, "n");
    m_keyList.insert(MainWindow::Tool, "t");
    m_keyList.insert(MainWindow::Tool_Add, "a");
    m_keyList.insert(MainWindow::Tool_Select, "s");
    m_keyList.insert(MainWindow::Tool_Move, "m");
    fillCommandList();

    QList<QString> uniqueKeys;
    QList<QString> keys = m_keyList.values();
    for (int i = 0; i < keys.size(); i++) {
        if (!uniqueKeys.contains(keys[i])) {
            uniqueKeys.append(keys[i]);
            qDebug() << "Added to key list: " << keys[i];
        }
    }


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


    Q_ASSERT(b);

}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_signalMapper;
    for (int i = 0; i < m_shortcutList.length(); i++) {
        delete m_shortcutList[i];
    }
}


void MainWindow::displayHelp(QPainter &davinci)
{
    QRect content(10, 10, 400, 200);
    QRect text(content.topLeft() + QPoint(10, 10), content.size() - QSize(20,20));
    davinci.fillRect(content, QColor(0,0,40, 200));
    QString helpText;
    for (int i = 0; i < m_commands.size(); i++) {
        helpText.append(m_commands.at(i) + "\n");
    }
    davinci.drawText(text, helpText);
}


void MainWindow::shortcutUsed(QString which)
{
    TimedShortcut ts;
    ts.shortcut = which;
    ts.start = QTime::currentTime();

    qDebug() << which << " pressed. Last shortcut: " << m_lastShortcut.start.elapsed();

//    QString at = QString(" @ %1.%2::%3")
//            .arg(ts.start.minute())
//            .arg(ts.start.second())
//            .arg(ts.start.msec());

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
            }
        }
        else if (m_lastShortcut.shortcut == m_keyList[MainWindow::Delete]) {
            if (which == m_keyList[MainWindow::Delete_Node]) {
                emit deleteNodes();
            }
        }
        else if (m_lastShortcut.shortcut == m_keyList[MainWindow::Tool]) {
            if (which == m_keyList[MainWindow::Tool_Add]) {
                emit setMode(Canvas::ToolMode_Add);
            } else if (which == m_keyList[MainWindow::Tool_Select]) {
                emit setMode(Canvas::ToolMode_Select);
            } else if (which == m_keyList[MainWindow::Tool_Move]) {
                emit setMode(Canvas::ToolMode_Move);
            }
        }
    } else {
        if (which == m_keyList[MainWindow::Abort]) {
            emit abort(Canvas::Abort_General);
        } else {
            qDebug() << "(Shortcut timed out.)";
        }
    }
    if (which == m_keyList[MainWindow::Help]) {
        m_wCanvas->toggleHelp();
    }

    m_lastShortcut = ts;
}
