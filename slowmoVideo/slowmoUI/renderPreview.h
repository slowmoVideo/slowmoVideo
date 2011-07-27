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

namespace Ui {
    class RenderPreview;
}
class Project_sV;
class QMainWindow;

class RenderPreview : public QWidget
{
    Q_OBJECT

public:
    explicit RenderPreview(Project_sV *project, QWidget *parent = 0);
    ~RenderPreview();

    void load(Project_sV *project);

public slots:
    void slotRenderAt(float time);

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
