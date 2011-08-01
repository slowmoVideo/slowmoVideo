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

//#define DEBUG_N

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
Node_sV::Node_sV(const QPointF &point) :
    m_x(point.x()),
    m_y(point.y())
{
    init();
}
Node_sV::Node_sV(const Node_sV &other) :
    m_x(other.x()),
    m_y(other.y()),
    m_leftHandle(other.m_leftHandle),
    m_rightHandle(other.m_rightHandle),
    m_shutterFunctionID(other.m_shutterFunctionID)
{
    init();
    m_leftCurveType = other.m_leftCurveType;
    m_rightCurveType = other.m_rightCurveType;
    Q_ASSERT(this == m_leftHandle.parentNode());
    Q_ASSERT(this == m_rightHandle.parentNode());
}

void Node_sV::init()
{
    m_moveX = 0;
    m_moveY = 0;
    m_selected = false;
    m_leftHandle.setParentNode(this);
    m_rightHandle.setParentNode(this);
    m_leftCurveType = CurveType_Linear;
    m_rightCurveType = CurveType_Linear;
    Q_ASSERT(this == m_leftHandle.parentNode());
    Q_ASSERT(this == m_rightHandle.parentNode());
}



////////// Basic commands

qreal Node_sV::x() const { return m_x + m_moveX; }
qreal Node_sV::y() const { return m_y + m_moveY; }
qreal Node_sV::xUnmoved() const { return m_x; }
qreal Node_sV::yUnmoved() const { return m_y; }

qreal Node_sV::setX(qreal x) { qreal ret = m_x; m_x = x; return ret; }
qreal Node_sV::setY(qreal y) { qreal ret = m_y; m_y = y; return ret; }

void Node_sV::select(bool select) { m_selected = select; }
bool Node_sV::selected() const { return m_selected; }



////////// Curve types, handles

const NodeHandle_sV& Node_sV::leftNodeHandle() const { return m_leftHandle; }
const NodeHandle_sV& Node_sV::rightNodeHandle() const { return m_rightHandle; }
CurveType Node_sV::leftCurveType() const { return m_leftCurveType; }
CurveType Node_sV::rightCurveType() const { return m_rightCurveType; }
const QString Node_sV::shutterFunctionID() const { return m_shutterFunctionID; }

void Node_sV::setLeftCurveType(CurveType type) { m_leftCurveType = type; }
void Node_sV::setRightCurveType(CurveType type) { m_rightCurveType = type; }
void Node_sV::setLeftNodeHandle(qreal x, qreal y) {
    Q_ASSERT(x <= 0); // Relative offset to current node; ensure that mapping is injective
    m_leftHandle.rx() = x; m_leftHandle.ry() = y;
}
void Node_sV::setRightNodeHandle(qreal x, qreal y) {
    Q_ASSERT(x >= 0);
    m_rightHandle.rx() = x; m_rightHandle.ry() = y;
}
void Node_sV::setShutterFunctionID(QString id)
{
    m_shutterFunctionID = id;
}



////////// Movement

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




////////// Operators

bool Node_sV::operator <(const Node_sV& other) const
{
    return m_x < other.x();
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
void Node_sV::operator =(const Node_sV& other)
{
    if (this != &other) {
#ifdef DEBUG_N
        qDebug() << "Other: " << other;
#endif
        m_x = other.m_x;
        m_y = other.m_y;
        m_leftHandle.setX(other.leftNodeHandle().x());
        m_leftHandle.setY(other.leftNodeHandle().y());
        m_rightHandle.setX(other.rightNodeHandle().x());
        m_rightHandle.setY(other.rightNodeHandle().y());
        m_shutterFunctionID = other.m_shutterFunctionID;
#ifdef DEBUG_N
        qDebug() << "This: " << *this;
#endif
    }
    Q_ASSERT(this == m_leftHandle.parentNode());
    Q_ASSERT(this == m_rightHandle.parentNode());
}



////////// Conversion

QPointF Node_sV::toQPointF() const
{
    return QPointF(x(), y());
}

QDebug operator<<(QDebug qd, const Node_sV& n)
{
    qd.nospace() << "(";
    qd.nospace() << n.x() << "|" << n.y();
    if (n.selected()) { qd.nospace() << "|s"; }
    qd.nospace() << ")@" << &n << " l: " << n.leftNodeHandle() << ", r: " << n.rightNodeHandle() << "\n";
    return qd.maybeSpace();
}
