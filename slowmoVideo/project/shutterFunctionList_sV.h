/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SHUTTERFUNCTIONLIST_SV_H
#define SHUTTERFUNCTIONLIST_SV_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QRegExp>
class NodeList_sV;
class ShutterFunction_sV;

class ShutterFunctionList_sV
{
public:
    ShutterFunctionList_sV(NodeList_sV *nodes);
    ~ShutterFunctionList_sV();

    static QRegExp regexID;

    int size() const;
    const QString nextID() const;
    bool updateID(ShutterFunction_sV *function, const QString id);

    // can return NULL!
    ShutterFunction_sV* function(const QString id);
    const ShutterFunction_sV* at(int index) const;
    ShutterFunction_sV* addFunction(const ShutterFunction_sV function, bool generateID);
    // todo: update references on nodes
    bool removeFunction(const QString id);

private:
    QList<ShutterFunction_sV*> m_functions;
    NodeList_sV *m_nodes;
};

#endif // SHUTTERFUNCTIONLIST_SV_H
