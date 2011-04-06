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
    m_y(y),
    m_selected(false)
{

}

qreal Node::x() const { return m_x; }
qreal Node::y() const { return m_y; }

void Node::select(bool select)
{
    m_selected = select;
}
bool Node::selected() const { return m_selected; }

bool Node::operator <(const Node& other) const
{
    return m_x < other.x();
}
bool Node::operator ==(const Node& other) const
{
    return m_x == other.x() && m_y == other.y();
}

QDebug operator<<(QDebug qd, const Node& n)
{
    qd.nospace() << "(";
    qd.nospace() << n.x() << "|" << n.y();
    if (n.selected()) { qd.nospace() << "|s"; }
    qd.nospace() << ")";
    return qd.maybeSpace();
}
