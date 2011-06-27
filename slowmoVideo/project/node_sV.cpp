/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "node_sV.h"
#include <QDebug>

Node_sV::Node_sV() :
    m_x(0),
    m_y(0)
{
    init();
}

Node_sV::Node_sV(const qreal &x, const qreal &y) :
    m_x(x),
    m_y(y)
{
    init();
}

void Node_sV::init()
{
    m_moveX = 0;
    m_moveY = 0;
    m_selected = false;
    m_leftCurveType = CurveType_Linear;
    m_rightCurveType = CurveType_Linear;
}

qreal Node_sV::x() const { return m_x + m_moveX; }
qreal Node_sV::y() const { return m_y + m_moveY; }
qreal Node_sV::xUnmoved() const { return m_x; }
qreal Node_sV::yUnmoved() const { return m_y; }

qreal Node_sV::setX(qreal x) { qreal ret = m_x; m_x = x; return ret; }
qreal Node_sV::setY(qreal y) { qreal ret = m_y; m_y = y; return ret; }

void Node_sV::select(bool select) { m_selected = select; }
bool Node_sV::selected() const { return m_selected; }

const Node_sV::NodeHandle_sV& Node_sV::leftNodeHandle() const { return m_leftHandle; }
const Node_sV::NodeHandle_sV& Node_sV::rightNodeHandle() const { return m_rightHandle; }
Node_sV::CurveType Node_sV::leftCurveType() const { return m_leftCurveType; }
Node_sV::CurveType Node_sV::rightCurveType() const { return m_rightCurveType; }

void Node_sV::setLeftCurveType(CurveType type) { m_leftCurveType = type; }
void Node_sV::setRightCurveType(CurveType type) { m_rightCurveType = type; }
void Node_sV::setLeftNodeHandle(qreal x, qreal y) {
    Q_ASSERT(x <= 0); // Relative offset to current node; ensure that mapping is injective
    m_leftHandle.x = x; m_leftHandle.y = y;
}
void Node_sV::setRightNodeHandle(qreal x, qreal y) {
    Q_ASSERT(x >= 0);
    m_rightHandle.x = x; m_rightHandle.y = y;
}

bool Node_sV::operator <(const Node_sV& other) const
{
    return m_x < other.x();
}

void Node_sV::move(const Node_sV &dist)
{
    m_moveX = dist.x();
    m_moveY = dist.y();
}
void Node_sV::abortMove()
{
    m_moveX = 0;
    m_moveY = 0;
}
void Node_sV::confirmMove()
{
    m_x += m_moveX;
    m_y += m_moveY;
    m_moveX = 0;
    m_moveY = 0;
}

bool Node_sV::operator ==(const Node_sV& other) const
{
    return m_x == other.m_x && m_y == other.m_y
            && m_moveX == other.m_moveX && m_moveY == other.m_moveY;
}
Node_sV Node_sV::operator -(const Node_sV& other) const
{
    return Node_sV(m_x - other.m_x, m_y - other.m_y);
}
Node_sV Node_sV::operator +(const Node_sV& other) const
{
    return Node_sV(m_x + other.m_x, m_y + other.m_y);
}
void Node_sV::operator +=(const Node_sV& other)
{
    m_x += other.m_x;
    m_y += other.m_y;
}
void Node_sV::operator -=(const Node_sV& other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
}

QDebug operator<<(QDebug qd, const Node_sV& n)
{
    qd.nospace() << "(";
    qd.nospace() << n.x() << "|" << n.y();
    if (n.selected()) { qd.nospace() << "|s"; }
    qd.nospace() << ")";
    return qd.maybeSpace();
}
