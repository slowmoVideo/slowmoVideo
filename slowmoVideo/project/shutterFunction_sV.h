/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SHUTTERFUNCTION_SV_H
#define SHUTTERFUNCTION_SV_H

#include <QtCore/QString>
#include <QtCore/QPointF>
#include <QtScript/QScriptValue>
class QScriptEngine;

class ShutterFunction_sV
{
public:
    ShutterFunction_sV();
    ShutterFunction_sV(const QString& function);
    ShutterFunction_sV(const ShutterFunction_sV& other);
    ~ShutterFunction_sV();

    static QString templateHeader;
    static QString templateBody;
    static QString templateFooter;

    void setID(const QString id);
    void updateFunction(const QString& function);

    QString id() const;
    QString function() const;
    /**
      \param x x location between two nodes, scaled to <pre>[0,1]</pre>
      \param t Output time
      \param fps Frames per second
      \param y Source frame at position x
      \param dy y change to the next frame
      */
    float evaluate(const float x, const float t, const float fps, const float y, const float dy);

    void operator =(const ShutterFunction_sV &other);

private:
    QString m_id;
    QString m_function;

    QScriptEngine *m_scriptEngine;
    QScriptValue m_compiledFunction;

    void init();
};

#endif // SHUTTERFUNCTION_SV_H
