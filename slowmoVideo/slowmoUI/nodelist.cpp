/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "nodelist.h"
#include "node.h"

#include <cmath>

#include <QDebug>

NodeList::NodeList(float minDist) :
    m_list(),
    m_minDist(minDist)
{
}

bool NodeList::add(const Node &node)
{
    bool add = true;

    uint pos = find(node.x());
    if (m_list.size() > pos) {
        add = fabs(node.x()-m_list.at(pos).x()) > m_minDist;
        qDebug() << "Left distance is " << fabs(node.x()-m_list.at(pos).x());
        if (add && m_list.size() > pos+1) {
            add = fabs(node.x()-m_list.at(pos+1).x()) > m_minDist;
            qDebug() << "Right distance is " << fabs(node.x()-m_list.at(pos+1).x());
        }
    }
    qDebug() << "Adding? " << add;
    if (add) {
        m_list.append(node);
        qSort(m_list);
    }
}

uint NodeList::deleteSelected()
{
    uint counter = 0;
    for (int i = 0; i < m_list.size(); ) {
        if (m_list.at(i).selected()) {
            m_list.removeOne(m_list.at(i));
            counter++;
        } else {
            i++;
        }
    }
    return counter;
}

void NodeList::unselectAll()
{
    for (int i = 0; i < m_list.size(); i++) {
        m_list[i].select(false);
    }
}


bool NodeList::validate() const
{
    bool valid = true;
    qreal last = -m_minDist;
    for (int i = 0; i < m_list.size() && valid; i++) {
        valid = m_list.at(i).x() >= 0
                && m_list.at(i).x() - last >= m_minDist;
        last = m_list.at(i).x();
        Q_ASSERT(valid);
    }
    return valid;
}


////////// Moving

void NodeList::moveSelected(const Node &time)
{
    qreal maxRMove = 1000;
    qreal maxLMove = -1000;
    const Node *left = NULL;
    const Node *right;
    for (uint i = 0; i < m_list.size(); i++) {
        right = &m_list.at(i);

        if (left != NULL) {
            if (left->selected() && !right->selected()) {
                // Move-right distance
                maxRMove = qMin(maxRMove, right->xUnmoved() - left->xUnmoved());
            } else if (!left->selected() && right->selected()) {
                // Move-left distance
                maxLMove = qMax(maxLMove, left->xUnmoved() - right->xUnmoved());
            }
        }

        left = right;
    }
    if (m_list.size() > 0) {
        // Do not allow to move nodes to x < 0
        maxLMove = qMax(maxLMove, -m_list.at(0).xUnmoved());
    }
    qDebug() << "Max move: left " << maxLMove << ", right: " << maxRMove;
    if (maxLMove <= time.x() && time.x() <= maxRMove) {
        for (uint i = 0; i < m_list.size(); i++) {
            if (m_list.at(i).selected()) {
                m_list[i].move(time);
            }
        }
    } else {
        qDebug() << "Not within valid range:" << time;
    }
}
void NodeList::shift(qreal after, qreal by)
{
    int pos = nodeAfter(after);
    if (pos >= 0) {
        if (pos > 0) {
            qDebug() << "Max of " << by << " and " << m_list.at(pos-1).xUnmoved() - m_list.at(pos).xUnmoved() + m_minDist;
            by = qMax(by, m_list.at(pos-1).xUnmoved() - m_list.at(pos).xUnmoved() + m_minDist);
        }
        if (pos == 0) {
            by = qMax(by, -m_list.at(pos).xUnmoved());
        }
        for (; pos < m_list.size(); pos++) {
            m_list[pos].move(Node(by, 0));
        }
    }
    if (!validate()) {
        qDebug() << "Invalid node configuration! (This should not happen.)";
    }
}

void NodeList::confirmMove()
{
    for (uint i = 0; i < m_list.size(); i++) {
        m_list[i].confirmMove();
    }
}
void NodeList::abortMove()
{
    for (uint i = 0; i < m_list.size(); i++) {
        if (m_list.at(i).selected()) {
            m_list[i].abortMove();
        }
    }
}




////////// Access

uint NodeList::find(qreal time) const
{
    uint pos;
    for (
         pos = 0;
         m_list.size() > (pos+1) && m_list.at(pos+1).x() <= time;
         pos++
         ) {}
    return pos;
}

int NodeList::nodeAfter(qreal time) const
{
    int pos = 0;
    while (m_list.size() > pos) {
        if (m_list.at(pos).xUnmoved() >= time) {
            break;
        }
        pos++;
    }
    if (pos >= m_list.size()) {
        pos = -1;
    }
    Q_ASSERT(pos < 0 || m_list.at(pos).xUnmoved() >= time);
    return pos;
}
const Node* NodeList::at(qreal t) const
{
    for (int i = 0; i < m_list.size(); i++) {
        if (fabs(m_list.at(i).x() - t) < m_minDist) {
            return &m_list.at(i);
        }
    }
    return NULL;
}
const Node& NodeList::at(uint i) const { return m_list.at(i); }
Node& NodeList::operator[](int i) { return m_list[i]; }
int NodeList::size() const { return m_list.size(); }

QDebug operator<<(QDebug dbg, const NodeList &list)
{
    for (uint i = 0; i < list.size(); i++) {
        dbg.nospace() << list.at(i) << " ";
    }
    return dbg.maybeSpace();
}
