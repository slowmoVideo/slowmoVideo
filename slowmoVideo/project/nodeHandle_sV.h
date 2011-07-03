/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NODEHANDLE_SV_H
#define NODEHANDLE_SV_H

#include <QtCore/QPointF>

class Node_sV;

class NodeHandle_sV : public QPointF
{
public:
    NodeHandle_sV();
    NodeHandle_sV(qreal x, qreal y);
    NodeHandle_sV(const QPointF &other);

    void setParentNode(Node_sV *node);
    Node_sV* parentNode();


private:
    Node_sV *m_parentNode;
};

#endif // NODEHANDLE_SV_H
