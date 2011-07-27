/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef INPUTMONITOR_H
#define INPUTMONITOR_H

#include <QWidget>
#include <QSemaphore>

namespace Ui {
    class InputMonitor;
}

class QImage;
class InputMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit InputMonitor(QWidget *parent = 0);
    ~InputMonitor();

protected:
    virtual void paintEvent(QPaintEvent *event);


public slots:
    void slotLoadImage(const QString &filename);

private:
    Ui::InputMonitor *ui;

    QSemaphore m_semaphore;
    QString *m_queue[2];
};

#endif // INPUTMONITOR_H
