/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NODELIST_H
#define NODELIST_H

#include <QList>
#include <QtGlobal>

class Node;

class NodeList
{
public:
    NodeList(float minDist = 1/30.0f);
    /**
      Add a new node at the given position.
      @return true if the node has been added. The node is NOT added
      if it is too close to another node.
      */
    bool add(const Node &node);
    uint deleteSelected();

    void unselectAll();

    void moveSelected(const Node &time);
    void confirmMove();
    void abortMove();


    uint find(qreal time) const;

    /**
      @return A pointer to the node at the given time (or not further
      than minDist away), or NULL if there is no node at t.
      */
    const Node* at(qreal t) const;
    const Node& at(uint i) const;
    Node& operator[](int i);
    int size() const;

private:
    QList<Node> m_list;
    const float m_minDist;
};

QDebug operator<<(QDebug qd, const NodeList &list);

#endif // NODELIST_H
