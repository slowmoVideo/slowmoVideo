/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef XMLPROJECTRW_SV_H
#define XMLPROJECTRW_SV_H

#include "../lib/defs_sV.hpp"
#include "tag_sV.h"

#include <QtXml>

class Node_sV;
class Project_sV;
class AbstractFrameSource_sV;

// Remember to change the slowmoVideo version as well
#define SLOWMOPROJECT_VERSION_MAJOR 2
#define SLOWMOPROJECT_VERSION_MINOR 5

/**
  \brief Reads and writes project files in XML format

  Additional stored parameters require a minor version change.
  Big changes in the schema, like elements moved to a different node, require
  a major version change and a function that loads old project files.

  Version changes (both major and minor) require a micro slowmoVideo version change.
  */
class XmlProjectRW_sV
{
public:
    /**
      \fn loadProject()
      Reads an XML project file.
      \param filename Project file to load
      \param warning Information message when trying to load project files with a wrong version number
      \return NULL if an error ocurred.
      */
    /**
      \fn saveProject()
      Saves a project to an XML project file.
      */

    static Project_sV* loadProject(QString filename, QString *warning = NULL) throw(FrameSourceError, Error_sV);
    static int saveProject(Project_sV *project, QString filename) throw(Error_sV);

private:
    static const QDomElement nodeToDom(QDomDocument *doc, const Node_sV *node);
    static const QDomElement tagToDom(QDomDocument *doc, const Tag_sV &tag);

    static const QDomElement frameSource(QDomDocument *doc, const AbstractFrameSource_sV *frameSource);
    static void loadFrameSource(QXmlStreamReader *reader, Project_sV *project) throw(FrameSourceError);
};

#endif // XMLPROJECTRW_SV_H
