/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef RENDERINGDIALOG_H
#define RENDERINGDIALOG_H

#include <QDialog>

namespace Ui {
    class RenderingDialog;
}

class QButtonGroup;
class RenderTask_sV;
class Project_sV;

/**
  \brief Dialog for rendering option
  */
class RenderingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderingDialog(Project_sV *project, QWidget *parent = 0);
    ~RenderingDialog();

    /** \return \c NULL on invalid input, a render task for the given project otherwise */
    RenderTask_sV* buildTask();

public slots:
    bool slotValidate();

private:
    Ui::RenderingDialog *ui;

    Project_sV *m_project;
    QButtonGroup *m_targetGroup;
    QButtonGroup *m_sectionGroup;
    QButtonGroup *m_blurGroup;

    void fillTagLists();

private slots:
    void slotBrowseImagesDir();
    void slotBrowseVideoFile();
    void slotUpdateRenderTarget();

    void slotSectionModeChanged();
    void slotTagIndexChanged();

    void slotSaveSettings();
    void slotClearFlowCache();

};

#endif // RENDERINGDIALOG_H
