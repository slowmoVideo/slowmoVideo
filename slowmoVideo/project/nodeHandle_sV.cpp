/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "nodeHandle_sV.h"

NodeHandle_sV::NodeHandle_sV() :
    QPointF(0, 0),
    m_parentNode(NULL)
{
}

NodeHandle_sV::NodeHandle_sV(const QPointF &other) :
    QPointF(other),
    m_parentNode(NULL)
{
}

NodeHandle_sV::NodeHandle_sV(qreal x, qreal y) :
    QPointF(x, y),
    m_parentNode(NULL)
{
}

NodeHandle_sV::NodeHandle_sV(const NodeHandle_sV &other) :
    QPointF(other)
{
}

void NodeHandle_sV::setParentNode(Node_sV *node)
{
    m_parentNode = node;
}

const Node_sV* NodeHandle_sV::parentNode() const
{
    return m_parentNode;
}

QDebug operator <<(QDebug qd, const NodeHandle_sV& h)
{
    qd.nospace() << "(" << h.x() << "|" << h.y() << "):" << &h << "@" << h.parentNode();
    return qd.maybeSpace();
}
