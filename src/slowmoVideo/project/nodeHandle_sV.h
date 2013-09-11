/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NODEHANDLE_SV_H
#define NODEHANDLE_SV_H

#include "canvasObject_sV.h"
#include <QtCore/QPointF>
#include <QtCore/QDebug>

class Node_sV;

class NodeHandle_sV : public QPointF, public CanvasObject_sV
{
public:
    NodeHandle_sV();
    NodeHandle_sV(qreal x, qreal y);
    NodeHandle_sV(const QPointF &other);
    NodeHandle_sV(const NodeHandle_sV &other);
    ~NodeHandle_sV() {}

    void setParentNode(Node_sV *node);
    const Node_sV* parentNode() const;


private:
    Node_sV *m_parentNode;
};

QDebug operator<<(QDebug qd, const NodeHandle_sV& n);

#endif // NODEHANDLE_SV_H
