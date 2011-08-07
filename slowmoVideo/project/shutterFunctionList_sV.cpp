/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "shutterFunctionList_sV.h"
#include "shutterFunction_sV.h"
#include "nodeList_sV.h"

#include <QtCore/QDebug>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

QRegExp ShutterFunctionList_sV::regexID("^[\\d\\w]+$");

ShutterFunctionList_sV::ShutterFunctionList_sV(NodeList_sV *nodes) :
    m_nodes(nodes)
{
}

ShutterFunctionList_sV::~ShutterFunctionList_sV()
{
    for (int i = 0; i < m_functions.size(); i++) {
        delete m_functions.at(i);
    }
}

int ShutterFunctionList_sV::size() const
{
    return m_functions.size();
}

const QString ShutterFunctionList_sV::nextID() const
{
    int nr = 1;
    QString id;

    bool stop = false;
    while (!stop) {
        stop = true;
        id = QString("func%1").arg(nr);

        for (int i = 0; i < m_functions.size(); i++) {
            if (m_functions.at(i)->id() == id) {
                nr++;
                stop = false;
                continue;
            }
        }
    }

    return id;
}

bool ShutterFunctionList_sV::updateID(ShutterFunction_sV *function, const QString id)
{
    if (id.length() == 0 || regexID.indexIn(id) != 0) {
        qDebug() << "Not a valid ID: " << id;
        return false;
    }
    for (int i = 0; i < m_functions.size(); i++) {
        if (function != m_functions.at(i) && m_functions.at(i)->id() == id) {
            qDebug() << "ID already exists!";
            return false;
        }
    }
    function->setID(id);
    return true;
}

ShutterFunction_sV* ShutterFunctionList_sV::addFunction(const ShutterFunction_sV function, bool generateID)
{
    ShutterFunction_sV *fun = new ShutterFunction_sV(function);
    if (generateID) {
        fun->setID(nextID());
    }
    for (int i = 0; i < m_functions.size(); i++) {
        if (m_functions.at(i)->id() == fun->id()) {
            qDebug() << "Function ID is already here!";
            delete fun;
            Q_ASSERT(false);
            return NULL;
        }
    }
    m_functions.append(fun);
    return fun;
}

bool ShutterFunctionList_sV::removeFunction(const QString id)
{
    for (int i = 0; i < m_functions.size(); i++) {
        if (m_functions.at(i)->id() == id) {
            delete m_functions.at(i);
            m_functions.removeAt(i);

            for (int i = 0; i < m_nodes->size(); i++) {
                if (m_nodes->at(i).shutterFunctionID() == id) {
                    (*m_nodes)[i].setShutterFunctionID("");
                }
            }

            return true;
        }
    }
    return false;
}

const ShutterFunction_sV* ShutterFunctionList_sV::at(int index) const
{
    Q_ASSERT(index < m_functions.size());
    return m_functions.at(index);
}

ShutterFunction_sV* ShutterFunctionList_sV::function(const QString id)
{
    for (int i = 0; i < m_functions.size(); i++) {
        if (m_functions.at(i)->id() == id) {
            return m_functions[i];
        }
    }
    return NULL;
}
