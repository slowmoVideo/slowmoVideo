/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef RENDERPREVIEW_H
#define RENDERPREVIEW_H

#include <QWidget>
#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>
#include "../lib/defs_sV.hpp"

namespace Ui {
    class RenderPreview;
}
class Project_sV;
class QMainWindow;

/**
  \brief Renders a preview frame from the project
  */
class RenderPreview : public QWidget
{
    Q_OBJECT

public:
    explicit RenderPreview(Project_sV *project, QWidget *parent = 0);
    ~RenderPreview();

    /// Uses the given project (and its curve etc.)
    void load(Project_sV *project);

public slots:
    /// Renders the output frame at the given time (asynchronous)
    void slotRenderAt(qreal time);

private:
    Ui::RenderPreview *ui;
    Project_sV *m_project;

    QMainWindow *m_parentMainWindow;

    QFutureWatcher<QImage> m_futureWatcher;
    QFuture<QImage> m_future;

    void notify(const QString message);

private slots:
    void slotUpdateImage();
};

#endif // RENDERPREVIEW_H
