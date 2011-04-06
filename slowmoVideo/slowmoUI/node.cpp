/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "node.h"
#include <QDebug>

Node::Node() :
    m_x(0),
    m_y(0)
{
}

Node::Node(const qreal &x, const qreal &y) :
    m_x(x),
    m_y(y)
{

}

qreal Node::x() const { return m_x; }
qreal Node::y() const { return m_y; }

bool Node::operator <(const Node& other) const
{
    return m_x < other.x();
}

QDebug operator<<(QDebug qd, const Node& n)
{
    qd.nospace() << "(" << n.x() << "|" << n.y() << ")";
    return qd.maybeSpace();
}
