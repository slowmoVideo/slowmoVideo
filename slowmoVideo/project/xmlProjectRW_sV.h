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
#include "abstractProjectRW_sV.h"
#include "tag_sV.h"

#include <QtXml>

class Node_sV;
class AbstractFrameSource_sV;

class XmlProjectRW_sV// : public AbstractProjectRW_sV
{
public:
    /**
      Reads an XML project file.
      @return NULL if an error ocurred.
      */
    Project_sV* loadProject(QString filename) const throw(FrameSourceError);
    /**
      Saves a project to an XML project file.
      */
    int saveProject(Project_sV *project, QString filename) const;

private:
    static const QDomElement nodeToDom(QDomDocument *doc, const Node_sV *node);
    static const QDomElement tagToDom(QDomDocument *doc, const Tag_sV &tag);

    static const QDomElement frameSource(QDomDocument *doc, const AbstractFrameSource_sV *frameSource);
    static void loadFrameSource(QXmlStreamReader *reader, Project_sV *project) throw(FrameSourceError);
};

#endif // XMLPROJECTRW_SV_H
