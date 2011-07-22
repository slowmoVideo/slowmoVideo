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

#include <QtCore/QDebug>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

/// \todo Functions must be unique (ID), allow duplication and modification


ShutterFunctionList_sV::ShutterFunctionList_sV()
{
}

const QString ShutterFunctionList_sV::nextID()
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
    for (int i = 0; i < m_functions.size(); i++) {
        if (m_functions.at(i)->id() == id) {
            return false;
        }
    }
    function->setID(id);
    return true;
}

bool ShutterFunctionList_sV::addFunction(ShutterFunction_sV *function, bool generateID)
{
    if (m_functions.contains(function)) {
        qDebug() << "Function already is in this list.";
        Q_ASSERT(false);
        return false;
    }
    if (generateID) {
        function->setID(nextID());
    }
    m_functions.append(function);
    return true;
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
