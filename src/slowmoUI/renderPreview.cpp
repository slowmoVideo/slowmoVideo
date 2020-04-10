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
#include "project/emptyFrameSource_sV.h"
#include "project/project_sV.h"
#include "project/projectPreferences_sV.h"

#include <QtCore>
#include <QtGui/QPainter>
#include <QMainWindow>
#include <QStatusBar>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent>
#endif

RenderPreview::RenderPreview(Project_sV *project, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RenderPreview),
    m_project(project)
{
    ui->setupUi(this);

    m_parentMainWindow = dynamic_cast<QMainWindow*>(parentWidget());

    ui->info->setVisible(m_parentMainWindow == NULL);
    ui->info->clear();

    connect(&m_futureWatcher, SIGNAL(finished()), this, SLOT(slotUpdateImage()));
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

void RenderPreview::slotRenderAt(qreal time)
{
    if (dynamic_cast<EmptyFrameSource_sV*>(m_project->frameSource()) != NULL) {
        notify(tr("Cannot render preview, no frames loaded."));
        return;
    }
    if (m_project->nodes()->size() < 2) {
        notify(tr("Cannot render preview at the curve position since no curve is available."));
        return;
    }
    if (time >= m_project->nodes()->startTime() && time <= m_project->nodes()->endTime()) {
        notify(tr("Rendering preview at output time %1 s (might take some time) ...").arg(time));

        if (m_future.isRunning()) {
            notify(tr("Preview is still being rendered."));
        } else {

            RenderPreferences_sV prefs;
            prefs.fps() = m_project->preferences()->renderFPS();
            prefs.interpolation = m_project->preferences()->renderInterpolationType();
            prefs.size = FrameSize_Orig;

            m_future = QtConcurrent::run(m_project, &Project_sV::render, time, prefs);
            m_futureWatcher.setFuture(m_future);
            if (m_future.isFinished()) {
                qDebug() << "qFuture has already finished! Manually calling update.";
                slotUpdateImage();
            }
        }
    } else {
        notify(tr("Cannot render at output time %1 s; Not within the curve.").arg(time));
    }
}

void RenderPreview::slotUpdateImage()
{
    qDebug() << "Updating preview image now. Saving as /tmp/renderPreview.jpg."; ///< \todo do not save anymore
    ui->imageDisplay->loadImage(m_future.result());
    ui->imageDisplay->image().save(QDir::tempPath () + "/renderPreview.jpg");
    repaint();
    notify(tr("Preview rendering finished."));
}
