/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "renderPreview.h"
#include "ui_renderPreview.h"
#include "project/project_sV.h"

#include <QtCore>
#include <QtGui/QPainter>
#include <QtGui/QMainWindow>
#include <QtGui/QStatusBar>

RenderPreview::RenderPreview(Project_sV *project, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RenderPreview),
    m_project(project)
{
    ui->setupUi(this);

    m_parentMainWindow = dynamic_cast<QMainWindow*>(parentWidget());

    ui->info->setVisible(m_parentMainWindow == NULL);
    ui->info->clear();


    bool b = true;
    b &= connect(&m_futureWatcher, SIGNAL(finished()), this, SLOT(slotUpdateImage()));
}

RenderPreview::~RenderPreview()
{
    delete ui;
}

void RenderPreview::load(Project_sV *project)
{
    m_project = project;
}

void RenderPreview::notify(const QString message)
{
    if (m_parentMainWindow != NULL) {
        m_parentMainWindow->statusBar()->showMessage(message, 5000);
    } else {
        ui->info->setText(message);
    }
}

void RenderPreview::slotRenderAt(float time)
{
    if (time >= m_project->nodes()->startTime() && time <= m_project->nodes()->endTime()) {
        notify(QString("Rendering preview at output time %1 s (might take some time) ...").arg(time));

        if (m_future.isRunning()) {
            notify("Preview is still being rendered.");
        } else {
            m_future = QtConcurrent::run(m_project, &Project_sV::render,
                                                   time, Fps_sV(24, 1), InterpolationType_ForwardNew, FrameSize_Orig);
            m_futureWatcher.setFuture(m_future);
        }
    } else {
        notify(QString("Cannot render at output time %1 s; Not within the curve.").arg(time));
    }
}

void RenderPreview::slotUpdateImage()
{
    ui->imageDisplay->loadImage(m_future.result());
    repaint();
}
