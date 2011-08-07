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

/**
  \brief Defines the shutter length over a node segment.

  The function is written in QtScript which is basically ECMAScript (known from JavaScript).
  Examples:

  \code
  // 180° shutter
  return 1/2*dy;
  \endcode

  \code
  // (replay speed)²
  return Math.pow(dy/dx, 2);
  \endcode

  \code
  // Variable declarations
  var dx = 1/fps;
  var speed = dy/dx;
  if (speed < 1) { speed = 0; }
  return speed;
  \endcode
*/
class ShutterFunction_sV
{
public:
    /// Creates a default shutter function (evaluates to 0)
    ShutterFunction_sV();
    /// Creates a shutter function from the given QtScript code
    ShutterFunction_sV(const QString& function);
    /// Copy constructor
    ShutterFunction_sV(const ShutterFunction_sV& other);
    /// Destructor
    ~ShutterFunction_sV();

    /// Default template header with some comments and the available variables
    static QString templateHeader;
    /// Example body,
    static QString templateBody;
    /// Default template footer
    static QString templateFooter;

    /// Does not do any checking/validating of the ID.
    void setID(const QString id);
    /// Compiles the given function code
    void updateFunction(const QString& function);

    /// Function's ID (empty by default)
    QString id() const;
    /// Code used by this function
    QString function() const;
    /**
      Evaluates the function with the given parameters.
      \param x x location between two nodes, scaled to <pre>[0,1]</pre>
      \param t Output time
      \param fps Frames per second
      \param y Source frame at position x
      \param dy y change to the next frame
      */
    float evaluate(const float x, const float t, const float fps, const float y, const float dy);

private:
    QString m_id;
    QString m_function;

    QScriptEngine *m_scriptEngine;
    QScriptValue m_compiledFunction;

    void init();
    void operator =(const ShutterFunction_sV &other);
};

#endif // SHUTTERFUNCTION_SV_H
