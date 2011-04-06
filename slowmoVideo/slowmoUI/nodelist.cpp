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

const Node& NodeList::at(int i) const { return m_list.at(i); }
Node& NodeList::operator[](int i) { return m_list[i]; }
int NodeList::size() const { return m_list.size(); }

QDebug operator<<(QDebug dbg, const NodeList &list)
{
    for (int i = 0; i < list.size(); i++) {
        dbg.nospace() << list.at(i) << " ";
    }
    return dbg.maybeSpace();
}
