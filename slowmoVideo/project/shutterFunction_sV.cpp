/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "shutterFunction_sV.h"

#include <QtCore/QDebug>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

QString ShutterFunction_sV::templateHeader(
        "// x:   on [0,1] \n"
        "// t:   output time \n"
        "// fps: Frames per second (1/dt) \n"
        "// y:   source time at position x \n"
        "// dy:  Source time delta to next output frame \n"
        "(function(x, t, fps, y, dy) \n"
        "{");
QString ShutterFunction_sV::templateBody(
        "  // Replace this with your function \n"
        "  return Math.sin(x*Math.PI);"
        );
QString ShutterFunction_sV::templateFooter(
        "})");


ShutterFunction_sV::ShutterFunction_sV()
{
    init();
    updateFunction(templateBody);
}
ShutterFunction_sV::ShutterFunction_sV(const QString &function)
{
    init();
    updateFunction(function);
}
ShutterFunction_sV::ShutterFunction_sV(const ShutterFunction_sV &other)
{
    init();
    m_id = other.m_id;
    updateFunction(other.m_function);
}

void ShutterFunction_sV::init()
{
    m_scriptEngine = new QScriptEngine();
    qDebug() << "Script engine initialized for function " << this;
}
ShutterFunction_sV::~ShutterFunction_sV()
{
    delete m_scriptEngine;
}

void ShutterFunction_sV::operator =(const ShutterFunction_sV &other)
{
    qDebug() << "Shutter functions should not be copied!";
    Q_ASSERT(false);
    if (this != &other) {
        m_id = other.m_id;
        m_function = other.m_function;
        m_compiledFunction = other.m_compiledFunction;
    }
}


void ShutterFunction_sV::setID(const QString id)
{
    m_id = id;
}
QString ShutterFunction_sV::id() const
{
    return m_id;
}
QString ShutterFunction_sV::function() const
{
    return m_function;
}



void ShutterFunction_sV::updateFunction(const QString &function)
{
    m_function = function;

    QString f = QString("%1%2%3").arg(templateHeader).arg(m_function).arg(templateFooter);
//    qDebug() << "===== Function is:\n" << f << "\n=====";

    m_compiledFunction = m_scriptEngine->evaluate(f);
}

float ShutterFunction_sV::evaluate(const float x, const float t, const float fps, const float y, const float dy)
{
    QScriptValueList args;
    args << x << t << fps << y << dy;
    QString result = m_compiledFunction.call(QScriptValue(), args).toString();

    float val;

    bool ok = false;
    if (result.length() > 0) {
        val = result.toFloat(&ok);
    }
    if (!ok) {
        val = 0;
//        qDebug() << "Error: Could not evaluate function " << m_id;
    } else {
//        qDebug() << "Evaluated at " << x0 << ": " << val;
    }
    return val;
}
