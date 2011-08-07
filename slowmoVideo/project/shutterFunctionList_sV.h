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

/**
  \brief Maintains a list of ShutterFunction_sV and ensures unique IDs.

  When a function is removed, the nodes in the project's NodeList_sV
  are scanned for the function's ID and then reset to the default ID
  if they used the deleted function.
  */
class ShutterFunctionList_sV
{
public:
    /// Links the ShutterFunctionList_sV to the given NodeList_sV
    ShutterFunctionList_sV(NodeList_sV *nodes);
    /// Destructor
    ~ShutterFunctionList_sV();

    /// For validating IDs (should be alphanumeric)
    static QRegExp regexID;

    /// Number of functions in this list
    int size() const;
    /// Returns the next unique function ID
    const QString nextID() const;
    /// Updates the function's ID. Returns \c false if \c id is already taken.
    bool updateID(ShutterFunction_sV *function, const QString id);

    /// Returns the function with the given \c id, or \c NULL if there is no function
    ShutterFunction_sV* function(const QString id);
    /// Returns the shutter function at the given \c index
    const ShutterFunction_sV* at(int index) const;

    /**
      Adds the given function.
      \param generateID If \c true, a new ID generated with nextID() will be used
             instead of the one set for the function.
      */
    ShutterFunction_sV* addFunction(const ShutterFunction_sV function, bool generateID);
    /**
      Removes the function and resets all references from nodes to this function.
      \return \c false if a function with the given \c id could not be found.
      */
    bool removeFunction(const QString id);

private:
    QList<ShutterFunction_sV*> m_functions;
    NodeList_sV *m_nodes;
};

#endif // SHUTTERFUNCTIONLIST_SV_H
