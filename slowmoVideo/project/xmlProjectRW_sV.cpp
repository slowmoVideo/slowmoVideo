/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "xmlProjectRW_sV.h"

#include "project_sV.h"
#include "nodelist_sV.h"
#include "videoFrameSource_sV.h"
#include "emptyFrameSource_sV.h"
#include "../lib/defs_sV.hpp"

#include <QDebug>
#include <QTextStream>
#include <QXmlQuery>


/// \todo project directory

int XmlProjectRW_sV::saveProject(Project_sV *project, QString filename) const
{
    QDomDocument doc;
    QDomElement root = doc.createElement("sVproject");
    root.setAttribute("version", "2");
    doc.appendChild(root);

    // File info
    QDomElement info = doc.createElement("info");
    root.appendChild(info);
    QDomElement appName = doc.createElement("appName");
    appName.appendChild(doc.createTextNode("slowmoVideo"));
    info.appendChild(appName);


    // Project Preferences
    QDomElement preferences = doc.createElement("preferences");
    root.appendChild(preferences);
    /// \todo rendering settings
//    QDomElement renderFPS = doc.createElement("renderFPS");
//    renderFPS.appendChild(doc.createTextNode(QString("%1").arg(project->fpsOut())));
//    preferences.appendChild(renderFPS);
//    QDomElement renderSize = doc.createElement("renderSize");
//    renderSize.appendChild(doc.createTextNode(enumStr(project->renderFrameSize())));
//    preferences.appendChild(renderSize);


    // Project Resources
    QDomElement resources = doc.createElement("resources");
    root.appendChild(resources);
    resources.appendChild(frameSource(&doc, project->frameSource()));
    QDomElement projectDir = doc.createElement("projectDir");
    projectDir.appendChild(doc.createTextNode(project->getDirectory(".", false).absolutePath()));
    resources.appendChild(projectDir);


    // Nodes
    QDomElement nodes = doc.createElement("nodes");
    root.appendChild(nodes);
    NodeList_sV *nodeList = project->nodes();
    for (int i = 0; i < nodeList->size(); i++) {
        nodes.appendChild(nodeToDom(&doc, &nodeList->at(i)));
    }

    // Tags
    QDomElement tags = doc.createElement("tags");
    root.appendChild(tags);
    for (int i = 0; i < project->tags()->size(); i++) {
        tags.appendChild(tagToDom(&doc, project->tags()->at(i)));
    }

    qDebug() << doc.toString(2);

    QFile outFile(filename);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot write file " << filename;
        Q_ASSERT(false);
    }
    QTextStream output(&outFile);
    doc.save(output, 4);
    output.flush();
    outFile.close();

    return 0;
}

const QDomElement XmlProjectRW_sV::nodeToDom(QDomDocument *doc, const Node_sV *node)
{
    QDomElement el = doc->createElement("node");
    QDomElement x = doc->createElement("x");
    QDomElement y = doc->createElement("y");
    QDomElement selected = doc->createElement("selected");
    el.appendChild(x);
    el.appendChild(y);
    el.appendChild(selected);
    x.appendChild(doc->createTextNode(QString("%1").arg(node->xUnmoved())));
    y.appendChild(doc->createTextNode(QString("%1").arg(node->yUnmoved())));
    selected.appendChild(doc->createTextNode(QString("%1").arg(node->selected())));
    return el;
}

const QDomElement XmlProjectRW_sV::tagToDom(QDomDocument *doc, const Tag_sV &tag)
{
    QDomElement el = doc->createElement("tag");
    QDomElement t = doc->createElement("time");
    QDomElement desc = doc->createElement("description");
    el.appendChild(t);
    el.appendChild(desc);
    t.appendChild(doc->createTextNode(QString("%1").arg(tag.time())));
    desc.appendChild(doc->createTextNode(tag.description()));
    return el;
}

const QDomElement XmlProjectRW_sV::frameSource(QDomDocument *doc, const AbstractFrameSource_sV *frameSource)
{
    QDomElement source = doc->createElement("frameSource");
    if (dynamic_cast<const VideoFrameSource_sV *>(frameSource) != NULL) {
        qDebug() << "Frame source is a video.";

        const VideoFrameSource_sV *vfs = dynamic_cast<const VideoFrameSource_sV *>(frameSource);
        source.setAttribute("type", "videoFile");
        QDomElement file = doc->createElement("inputFile");
        file.appendChild(doc->createTextNode(vfs->videoFile()));
        source.appendChild(file);

    } else if (dynamic_cast<const EmptyFrameSource_sV *>(frameSource) != NULL) {
        qDebug() << "Frame source is empty.";
        source.setAttribute("type", "empty");
    } else {
        qDebug() << "Unknown frame source type; cannot save!";
        source.setAttribute("type", "unknown");
        Q_ASSERT(false);
    }
    return source;
}

void XmlProjectRW_sV::loadFrameSource(QXmlStreamReader *reader, Project_sV *project)
{
    QStringRef frameSourceType = reader->attributes().value("type");
    if (frameSourceType.compare("videoFile") == 0) {
        while (reader->readNextStartElement()) {
            if (reader->name() == "inputFile") {
                VideoFrameSource_sV *frameSource = new VideoFrameSource_sV(project, reader->readElementText());
                project->loadFrameSource(frameSource);
            } else {
                qDebug() << "Unknown element in video frame source section: " << reader->name();
            }
        }
    } else if (frameSourceType.compare("empty")) {
        EmptyFrameSource_sV *frameSource = new EmptyFrameSource_sV(project);
        project->loadFrameSource(frameSource);
    } else {
        qDebug() << "Unknown frame source: " << frameSourceType << "; Cannot load!";
        // \todo throw exception
        Q_ASSERT(false);
    }
}

Project_sV* XmlProjectRW_sV::loadProject(QString filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot read file " << filename;
        Q_ASSERT(false);
    } else {

        QXmlStreamReader xml;
        xml.setDevice(&file);
        if (!xml.readNextStartElement()) {
            qDebug() << "Could not read " << filename;

        } else {
            if (xml.name() != "sVproject") {
                qDebug() << "Invalid project file (incorrect root element): " << filename;
            } else {

                int version = 0;
                QStringRef sVersion = xml.attributes().value("version");
                if (!sVersion.isEmpty()) {
                    version = sVersion.toString().toInt();
                }
                qDebug() << "Reading project file: version " << version;

                Project_sV *project = new Project_sV();

                while (xml.readNextStartElement()) {
                    if (xml.name() == "info") {
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "appName") {
                                qDebug() << "App name: " << xml.readElementText();
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else if (xml.name() == "resources") {
                        while (xml.readNextStartElement()) {
                            if (version < 2 && xml.name() == "inputFile") {
                                QString inFilename = xml.readElementText();
                                qDebug() << "Input file: " << inFilename;
                                project->setProjectDir(QFileInfo(filename).absolutePath());
                                project->loadFrameSource(new VideoFrameSource_sV(project, inFilename));
                            } else if (xml.name() == "frameSource") {
                                loadFrameSource(&xml, project);
                            } else if (xml.name() == "projectDir") {
                                project->setProjectDir(xml.readElementText());
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else if (xml.name() == "nodes") {
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "node") {
                                Node_sV node;
                                while (xml.readNextStartElement()) {
                                    if (xml.name() == "x") {
                                        node.setX(QVariant(xml.readElementText()).toFloat());
                                    } else if (xml.name() == "y") {
                                        node.setY(QVariant(xml.readElementText()).toFloat());
                                    } else if (xml.name() == "selected") {
                                        node.select(QVariant(xml.readElementText()).toBool());
                                    } else {
                                        xml.skipCurrentElement();
                                    }
                                }
                                project->nodes()->add(node);
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else if (xml.name() == "tags") {
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "tag") {
                                Tag_sV tag;
                                while (xml.readNextStartElement()) {
                                    if (xml.name() == "time") {
                                        tag.setTime(QVariant(xml.readElementText()).toFloat());
                                    } else if (xml.name() == "description") {
                                        tag.setDescription(xml.readElementText());
                                    } else {
                                        xml.skipCurrentElement();
                                    }
                                }
                                project->tags()->push_back(tag);
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else {
                        xml.skipCurrentElement();
                    }
                }

                return project;

            }
        }
        file.close();
    }
    return NULL;
}
