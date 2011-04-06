#include "nodelist.h"
#include "node.h"

#include <cmath>

#include <QDebug>

bool NodeList::add(const Node &node)
{
    bool add = true;

    uint pos = find(node.x());
    if (m_list.size() > pos) {
        add = fabs(node.x()-m_list.at(pos).x()) > 1/30.0f;
        qDebug() << "Left distance is " << fabs(node.x()-m_list.at(pos).x());
        if (add && m_list.size() > pos+1) {
            add = fabs(node.x()-m_list.at(pos+1).x()) > 1/30.0f;
            qDebug() << "Right distance is " << fabs(node.x()-m_list.at(pos+1).x());
        }
    }
    qDebug() << "Adding? " << add;
    if (add) {
        m_list.append(node);
        qSort(m_list);
    }
}

uint NodeList::find(qreal x)
{
    uint pos;
    for (
         pos = 0;
         m_list.size() > (pos+1) && m_list.at(pos+1).x() <= x;
         pos++
         ) {}
    return pos;
}

const Node& NodeList::at(int i) { return m_list.at(i); }
int NodeList::size() { return m_list.size(); }
