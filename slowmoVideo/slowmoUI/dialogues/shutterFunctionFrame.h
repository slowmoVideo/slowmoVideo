/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SHUTTERFUNCTIONFRAME_H
#define SHUTTERFUNCTIONFRAME_H

#include "project/shutterFunction_sV.h"
#include <QFrame>

/**
  \brief Renders a ShutterFunction_sV
  */
class ShutterFunctionFrame : public QFrame
{
    Q_OBJECT
public:
    ShutterFunctionFrame(QWidget * parent = 0, Qt::WindowFlags f = 0);

    void updateValues(float y, float dy);

public slots:
    void slotDisplayFunction(const QString &function);

protected slots:
    virtual void paintEvent(QPaintEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    ShutterFunction_sV m_function;
    int m_frameHeight;
    float m_y;
    float m_dy;

};

#endif // SHUTTERFUNCTIONFRAME_H
