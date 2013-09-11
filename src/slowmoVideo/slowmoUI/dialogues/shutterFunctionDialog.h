/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SHUTTERFUNCTIONDIALOG_H
#define SHUTTERFUNCTIONDIALOG_H

#include <QDialog>
class Project_sV;
class ShutterFunction_sV;

namespace Ui {
    class ShutterFunctionDialog;
}

/**
  \brief Manages ShutterFunction_sV for the current curve
  */
class ShutterFunctionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShutterFunctionDialog(Project_sV *project, QWidget *parent = 0);
    ~ShutterFunctionDialog();

    void loadProject(Project_sV *project);
    void setSegment(int segment);

public slots:
    void slotNodesUpdated();

protected slots:
    virtual void paintEvent(QPaintEvent *e);
    virtual void closeEvent(QCloseEvent *e);

private:
    static QString emptyFunction;

    Ui::ShutterFunctionDialog *ui;
    Project_sV *m_project;

    ShutterFunction_sV *m_currentFunction;

    float m_dy;
    float m_t0;

    int m_segment;

private slots:
    void slotUpdateNode();

    void slotFunctionTextChanged();
    void slotLoadSelectedFunction();

    void slotAddFunction();
    void slotRemoveFunction();

    void slotNextSegment();
    void slotPrevSegment();
};

#endif // SHUTTERFUNCTIONDIALOG_H
