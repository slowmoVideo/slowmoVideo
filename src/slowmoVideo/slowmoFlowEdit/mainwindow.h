/*
slowmoFlowEdit is a user interface for editing slowmoVideo's Optical Flow files.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class FlowEditCanvas;
namespace Ui {
    class MainWindow;
}


#include <QMainWindow>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif
#include <QSettings>

#include "../libgui/combinedShortcuts.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const CombinedShortcuts& shortcuts() const;

protected slots:
    void closeEvent(QCloseEvent *e);

private:
    enum Shortcuts {
        BOOST1, BOOST2, BOOST3, OPEN, SAVE, PREV, NEXT, HELP, QUIT
    };

    Ui::MainWindow *ui;
    CombinedShortcuts m_cs;

    FlowEditCanvas *m_canvas;

    QSettings m_settings;
    QString m_lastFlowFile;

    void updateTitle();
    void loadFlow(QString filename);

    void amplify(float val);

    QString nextFilename(QString originalName, int shift) const;


private slots:
    void slotOpenFlow();
    void slotSaveFlow();

    void slotNextFile();
    void slotPrevFile();
    void slotChangeFile(int shift);

    void slotShortcutUsed(int id);
    void slotShowShortcuts();
};

#endif // MAINWINDOW_H
