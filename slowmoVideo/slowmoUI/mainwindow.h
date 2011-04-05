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

namespace Ui {
    class MainWindow;
}

#include <QMap>
#include <QList>
#include <QTime>

class DispWidget;
class HelpText;
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
        Add,
        Add_Clip,
        Add_Color,
        Add_Text,
        Quit,
        Quit_Quit,
        Help,
        Help_Help
    };

private:
    Ui::MainWindow *ui;


    TimedShortcut m_lastShortcut;

    QSignalMapper *m_signalMapper;
    QList<QShortcut *> m_shortcutList;
    QMap<int, QString> m_keyList;

private slots:
    /**
      Handles timed shortcuts.
      Example: Pressing q twice quits the application iff the elapsed time
      between the two key strokes is small enough.
     */
    void shortcutUsed(QString);
};

#endif // MAINWINDOW_H
