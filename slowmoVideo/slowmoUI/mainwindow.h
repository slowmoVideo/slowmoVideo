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

#include <QMainWindow>
#include "canvas.h"
#include "../project/project_sV.h"

namespace Ui {
    class MainWindow;
}

#include <QMap>
#include <QList>
#include <QTime>

class Canvas;
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
        Tool_Add,
        Tool_Select,
        Tool_Move,
        Help
    };

    static void displayHelp(QPainter &davinci);
    static QStringList m_commands;

private:
    Ui::MainWindow *ui;


    TimedShortcut m_lastShortcut;

    QSignalMapper *m_signalMapper;
    QList<QShortcut *> m_shortcutList;
    QMap<int, QString> m_keyList;

    Canvas *m_wCanvas;

    Project_sV m_project;


    static void fillCommandList();

private slots:
    /**
      Handles timed shortcuts.
      Example: Pressing q twice quits the application iff the elapsed time
      between the two key strokes is small enough.
     */
    void shortcutUsed(QString);


signals:
    void deleteNodes();
    void setMode(const Canvas::ToolMode mode);
    void abort(const Canvas::Abort abort);

};

#endif // MAINWINDOW_H
