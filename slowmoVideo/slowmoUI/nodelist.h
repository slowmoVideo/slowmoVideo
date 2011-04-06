#ifndef NODELIST_H
#define NODELIST_H

#include <QList>
#include <QtGlobal>

class Node;

class NodeList
{
public:
    bool add(const Node &node);
    uint find(qreal x);

    const Node& at(int i);
    int size();

private:
    QList<Node> m_list;
};

#endif // NODELIST_H
