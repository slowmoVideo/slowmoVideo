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
class ShutterFunction_sV;

class ShutterFunctionList_sV
{
public:
    ShutterFunctionList_sV();

    const QString nextID();
    bool updateID(ShutterFunction_sV *function, const QString id);

    // can return NULL!
    ShutterFunction_sV* function(const QString id);
    bool addFunction(ShutterFunction_sV *function, bool generateID);

private:
    QList<ShutterFunction_sV*> m_functions;
};

#endif // SHUTTERFUNCTIONLIST_SV_H
