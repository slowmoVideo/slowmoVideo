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
#include "projectPreferences_sV.h"
#include "shutterFunctionList_sV.h"
#include "shutterFunction_sV.h"
#include "nodeList_sV.h"
#include "videoFrameSource_sV.h"
#include "emptyFrameSource_sV.h"
#include "imagesFrameSource_sV.h"
#include "motionBlur_sV.h"
#include "abstractFlowSource_sV.h"
//TODO: #include "defs_sV.hpp"

#include <QDebug>
#include <QTextStream>
#if 0
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QXmlQuery>
#endif // qt5 
#endif


int XmlProjectRW_sV::saveProject(Project_sV *project, QString filename) throw(Error_sV)
{
    //TODO: use global define in defs !
    QDomDocument doc;
    QDomElement root = doc.createElement("sVproject");
    root.setAttribute("version", SLOWMOPROJECT_VERSION_MAJOR);
    root.setAttribute("version_minor", SLOWMOPROJECT_VERSION_MINOR);
    doc.appendChild(root);

    // File info
    QDomElement info = doc.createElement("info");
    root.appendChild(info);
    QDomElement appName = doc.createElement("appName");
    appName.appendChild(doc.createTextNode("slowmoVideo"));
    QDomElement version = doc.createElement("version");
    version.setAttribute("major", SLOWMOVIDEO_VERSION_MAJOR);
    version.setAttribute("minor", SLOWMOVIDEO_VERSION_MINOR);
    version.setAttribute("micro", SLOWMOVIDEO_VERSION_PATCH);
    info.appendChild(appName);
    info.appendChild(version);


    ProjectPreferences_sV *pr = project->preferences();
    // Project Preferences
    QDomElement preferences = doc.createElement("preferences");
    root.appendChild(preferences);
    QDomElement renderSectionMode = doc.createElement("renderSectionMode");
    QDomElement renderStartTag = doc.createElement("renderStartTag");
    QDomElement renderEndTag = doc.createElement("renderEndTag");
    QDomElement renderStartTime = doc.createElement("renderStartTime");
    QDomElement renderEndTime = doc.createElement("renderEndTime");
    QDomElement renderFrameSize = doc.createElement("renderFrameSize");
    QDomElement renderInterpolation = doc.createElement("renderInterpolationType");
    QDomElement renderMotionblur = doc.createElement("renderMotionblurType");
    QDomElement renderFPS = doc.createElement("renderFPS");
    QDomElement renderSlowmoSamples = doc.createElement("renderSlowmoSamples");
    QDomElement renderMaxSamples = doc.createElement("renderMaxSamples");
    QDomElement renderTarget = doc.createElement("renderTarget");
    QDomElement imagesOutputDir = doc.createElement("imagesOutputDir");
    QDomElement imagesFilenamePattern = doc.createElement("imagesFilenamePattern");
    QDomElement videoFilename = doc.createElement("videoFilename");
    QDomElement videoCodec = doc.createElement("videoCodec");
    QDomElement flowV3dLambda = doc.createElement("flowV3dLambda");
    QDomElement prevTagAxis = doc.createElement("prevTagAxis");
    QDomElement viewport_t0 = doc.createElement("viewport_t0");
    QDomElement viewport_secRes = doc.createElement("viewport_secRes");
    QDomElement canvas_xAxisFPS = doc.createElement("canvas_xAxisFPS");
    preferences.appendChild(renderSectionMode);
    renderSectionMode.appendChild(renderStartTag);
    renderSectionMode.appendChild(renderEndTag);
    renderSectionMode.appendChild(renderStartTime);
    renderSectionMode.appendChild(renderEndTime);
    preferences.appendChild(renderFrameSize);
    preferences.appendChild(renderInterpolation);
    preferences.appendChild(renderMotionblur);
    preferences.appendChild(renderFPS);
    preferences.appendChild(renderSlowmoSamples);
    preferences.appendChild(renderMaxSamples);
    preferences.appendChild(renderTarget);
    preferences.appendChild(imagesOutputDir);
    preferences.appendChild(imagesFilenamePattern);
    preferences.appendChild(videoFilename);
    preferences.appendChild(videoCodec);
    preferences.appendChild(flowV3dLambda);
    preferences.appendChild(prevTagAxis);
    preferences.appendChild(viewport_t0);
    preferences.appendChild(viewport_secRes);
    preferences.appendChild(canvas_xAxisFPS);
    renderSectionMode.setAttribute("mode", pr->renderSectionMode());
    renderStartTag.setAttribute("label", pr->renderStartTag());
    renderEndTag.setAttribute("label", pr->renderEndTag());
    renderStartTime.setAttribute("time", pr->renderStartTime());
    renderEndTime.setAttribute("time", pr->renderEndTime());
    renderFrameSize.setAttribute("size", pr->renderFrameSize());
    renderInterpolation.setAttribute("type", pr->renderInterpolationType());
    renderMotionblur.setAttribute("type", pr->renderMotionblurType());
    renderFPS.setAttribute("fps", pr->renderFPS().toString());
    renderSlowmoSamples.setAttribute("number", project->motionBlur()->slowmoSamples());
    renderMaxSamples.setAttribute("number", project->motionBlur()->maxSamples());
    renderTarget.setAttribute("target", pr->renderTarget());
    imagesOutputDir.setAttribute("dir", pr->imagesOutputDir());
    imagesFilenamePattern.setAttribute("pattern", pr->imagesFilenamePattern());
    videoFilename.setAttribute("file", pr->videoFilename());
    videoCodec.setAttribute("codec", pr->videoCodec());
    flowV3dLambda.setAttribute("lambda", pr->flowV3DLambda());
    prevTagAxis.setAttribute("axis", QVariant(pr->lastSelectedTagAxis()).toString());
    viewport_t0.setAttribute("x", pr->viewport_t0().x());
    viewport_t0.setAttribute("y", pr->viewport_t0().y());
    viewport_secRes.setAttribute("x", QString().setNum(pr->viewport_secRes().x()));
    viewport_secRes.setAttribute("y", QString().setNum(pr->viewport_secRes().y()));
    canvas_xAxisFPS.setAttribute("fps", pr->canvas_xAxisFPS().toString());


    // Project Resources
    QDomElement resources = doc.createElement("resources");
    root.appendChild(resources);
    QDomElement projectDir = doc.createElement("projectDir");
    projectDir.appendChild(doc.createTextNode(project->getDirectory(".", false).absolutePath()));
    resources.appendChild(projectDir);
    resources.appendChild(frameSource(&doc, project->frameSource()));


    // Shutter functions
    QDomElement shutterFunctions = doc.createElement("shutterFunctions");
    root.appendChild(shutterFunctions);
    for (int i = 0; i < project->shutterFunctions()->size(); i++) {
        QDomElement func = doc.createElement("function");
        func.setAttribute("id", project->shutterFunctions()->at(i)->id());
        QDomElement code = doc.createElement("code");
        func.appendChild(code);
        code.appendChild(doc.createTextNode(project->shutterFunctions()->at(i)->function()));
        shutterFunctions.appendChild(func);
    }


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

    QFile outFile(filename);
    if (!outFile.open(QIODevice::WriteOnly)) {
        throw Error_sV(QObject::tr("Cannot write to %1; please check if you have write permissions.").arg(filename));
    }
    QTextStream output(&outFile);
    doc.save(output, 4);
    output.flush();
    outFile.close();

    project->setProjectFilename(filename);

    return 0;
}

const QDomElement XmlProjectRW_sV::nodeToDom(QDomDocument *doc, const Node_sV *node)
{
    QDomElement el = doc->createElement("node");
    QDomElement x = doc->createElement("x");
    QDomElement y = doc->createElement("y");
    QDomElement selected = doc->createElement("selected");
    QDomElement leftHandle = doc->createElement("leftHandle");
    QDomElement rightHandle = doc->createElement("rightHandle");
    QDomElement shutterFunc = doc->createElement("shutterFunction");
    QDomElement leftCurveType = doc->createElement("type");
    QDomElement rightCurveType = doc->createElement("type");
    QDomElement leftX = doc->createElement("x");
    QDomElement leftY = doc->createElement("y");
    QDomElement rightX = doc->createElement("x");
    QDomElement rightY = doc->createElement("y");
    el.appendChild(x);
    el.appendChild(y);
    el.appendChild(selected);
    el.appendChild(leftHandle);
    el.appendChild(rightHandle);
    if (node->shutterFunctionID().length() > 0) {
        el.appendChild(shutterFunc);
    }
    x.appendChild(doc->createTextNode(QString("%1").arg(node->xUnmoved())));
    y.appendChild(doc->createTextNode(QString("%1").arg(node->yUnmoved())));
    selected.appendChild(doc->createTextNode(QString("%1").arg(node->selected())));
    shutterFunc.appendChild(doc->createTextNode(node->shutterFunctionID()));

    leftHandle.appendChild(leftX);
    leftHandle.appendChild(leftY);
    leftHandle.appendChild(leftCurveType);
    rightHandle.appendChild(rightX);
    rightHandle.appendChild(rightY);
    rightHandle.appendChild(rightCurveType);

    leftX.appendChild(doc->createTextNode(QString("%1").arg(node->leftNodeHandle().x())));
    leftY.appendChild(doc->createTextNode(QString("%1").arg(node->leftNodeHandle().y())));
    leftCurveType.appendChild(doc->createTextNode(QVariant((int)node->leftCurveType()).toString()));
    rightX.appendChild(doc->createTextNode(QString("%1").arg(node->rightNodeHandle().x())));
    rightY.appendChild(doc->createTextNode(QString("%1").arg(node->rightNodeHandle().y())));
    rightCurveType.appendChild(doc->createTextNode(QVariant((int)node->rightCurveType()).toString()));

    return el;
}

const QDomElement XmlProjectRW_sV::tagToDom(QDomDocument *doc, const Tag_sV &tag)
{
    QDomElement el = doc->createElement("tag");
    QDomElement t = doc->createElement("time");
    QDomElement desc = doc->createElement("description");
    QDomElement axis = doc->createElement("axis");
    el.appendChild(t);
    el.appendChild(desc);
    el.appendChild(axis);
    t.appendChild(doc->createTextNode(QString("%1").arg(tag.time())));
    desc.appendChild(doc->createTextNode(tag.description()));
    axis.appendChild(doc->createTextNode(QVariant(tag.axis()).toString()));
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

    } else if (dynamic_cast<const ImagesFrameSource_sV *>(frameSource) != NULL) {
        qDebug() << "Frame source are images.";

        const ImagesFrameSource_sV *ifs = dynamic_cast<const ImagesFrameSource_sV *>(frameSource);
        source.setAttribute("type", "images");
        QDomElement files = doc->createElement("inputFiles");
        QStringList images = ifs->inputFiles();
        for (int i = 0; i < images.size(); i++) {
            QDomElement file = doc->createElement("file");
            file.appendChild(doc->createTextNode(images.at(i)));
            files.appendChild(file);
        }
        source.appendChild(files);

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

void XmlProjectRW_sV::loadFrameSource(QXmlStreamReader *reader, Project_sV *project) throw(FrameSourceError)
{
	//qDebug() << "loadFrameSource";
    QStringRef frameSourceType = reader->attributes().value("type");
    if (frameSourceType.compare(QLatin1String("videoFile")) == 0) {
        while (reader->readNextStartElement()) {
            if (reader->name() == "inputFile") {
                VideoFrameSource_sV *frameSource = new VideoFrameSource_sV(project, reader->readElementText());
                project->loadFrameSource(frameSource);
            } else {
                qDebug() << "Unknown element in video frame source section: " << reader->name();
                reader->skipCurrentElement();
            }
        }

    } else if (frameSourceType.compare(QLatin1String("images")) == 0) {
        while (reader->readNextStartElement()) {
            if (reader->name() == "inputFiles") {

                QStringList images;
                while (reader->readNextStartElement()) {
                    if (reader->name() == "file") {
                        images << reader->readElementText();
                    } else {
                        reader->skipCurrentElement();
                    }
                }
                ImagesFrameSource_sV *frameSource = new ImagesFrameSource_sV(project, images);
                project->loadFrameSource(frameSource);
            }
        }
    } else if (frameSourceType.compare(QLatin1String("empty")) == 0) {
        EmptyFrameSource_sV *frameSource = new EmptyFrameSource_sV(project);
        project->loadFrameSource(frameSource);
        reader->skipCurrentElement();
    } else {
        reader->skipCurrentElement();
        qDebug() << "Unknown frame source: " << frameSourceType << "; Cannot load!";
        throw FrameSourceError(QObject::trUtf8("Unknown frame source “%1”. Cannot load the project.").arg(frameSourceType.toString()));
    } 
    //qDebug() << "loadFrameSource ended";
}

Project_sV* XmlProjectRW_sV::loadProject(QString filename, QString *warning) throw(FrameSourceError, Error_sV)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot read file " << filename;
        throw Error_sV(QObject::tr("Cannot read from file %1. (Opening in read-only mode failed.)").arg(filename));
    } else {

        QXmlStreamReader xml;
        xml.setDevice(&file);
        if (!xml.readNextStartElement()) {
            qDebug() << "Could not read " << filename;
            throw Error_sV(QObject::tr("Invalid project file: %1").arg(filename));

        } else {
            if (xml.name() != "sVproject") {
                qDebug() << "Invalid project file (incorrect root element): " << filename;
            } else {

                int projVersionMajor = xml.attributes().value("version").toString().toInt();
                int projVersionMinor = xml.attributes().value("version_minor").toString().toInt();
                if (projVersionMajor > 0) {
                    qDebug().nospace() << "Reading project file: version " << projVersionMajor << "." << projVersionMinor;
                } else {
                    qDebug() << "Reading project file of unknown version";
                }

                int version_major = 0, version_minor = 0, version_micro = 0;

                Project_sV *project = new Project_sV();
                project->setProjectFilename(filename);
                project->setProjectDir(QFileInfo(filename).absolutePath());
                ProjectPreferences_sV *pr = project->preferences();

                while (xml.readNextStartElement()) {
                    if (xml.name() == "info") {
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "appName") {
                                qDebug() << "App name: " << xml.readElementText();
                            } else if (xml.name() == "version") {
                                version_major = xml.attributes().value("major").toString().toInt();
                                version_minor = xml.attributes().value("minor").toString().toInt();
                                version_micro = xml.attributes().value("micro").toString().toInt();
                                xml.skipCurrentElement();
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else if (xml.name() == "resources") {
                        while (xml.readNextStartElement()) {
                            if (projVersionMajor < 2 && xml.name() == "inputFile") {
                                QString inFilename = xml.readElementText();
                                qDebug() << "Input file: " << inFilename;
                                project->loadFrameSource(new VideoFrameSource_sV(project, inFilename));
                            } else if (xml.name() == "frameSource") {
                                loadFrameSource(&xml, project);
                            } else if (xml.name() == "projectDir") {
                                xml.skipCurrentElement();
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else if (xml.name() == "shutterFunctions") {
                        ShutterFunction_sV func;
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "function") {
                                func.setID(xml.attributes().value("id").toString());
                                while (xml.readNextStartElement()) {
                                    if (xml.name() == "code") {
                                        func.updateFunction(xml.readElementText());
                                    } else {
                                        xml.skipCurrentElement();
                                    }
                                }
                                project->shutterFunctions()->addFunction(func, false);
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
                                    } else if (xml.name() == "shutterFunction") {
                                        node.setShutterFunctionID(xml.readElementText());
                                    } else if (xml.name() == "leftHandle") {
                                        while (xml.readNextStartElement()) {
                                            QString text = xml.readElementText();
                                            if (xml.name() == "type") {
                                                node.setLeftCurveType((CurveType)text.toInt());
                                            } else if (xml.name() == "x") {
                                                node.setLeftNodeHandle(text.toDouble(), node.leftNodeHandle().y());
                                            } else if (xml.name() == "y") {
                                                node.setLeftNodeHandle(node.leftNodeHandle().x(), text.toDouble());
                                            } else {
                                                xml.skipCurrentElement();
                                            }
                                        }
                                    } else if (xml.name() == "rightHandle") {
                                        while (xml.readNextStartElement()) {
                                            QString text = xml.readElementText();
                                            if (xml.name() == "type") {
                                                node.setRightCurveType((CurveType)text.toInt());
                                            } else if (xml.name() == "x") {
                                                node.setRightNodeHandle(text.toDouble(), node.rightNodeHandle().y());
                                            } else if (xml.name() == "y") {
                                                node.setRightNodeHandle(node.rightNodeHandle().x(), text.toDouble());
                                            } else {
                                                xml.skipCurrentElement();
                                            }
                                        }
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
                                    } else if (xml.name() == "axis") {
                                        tag.setAxis((TagAxis)QVariant(xml.readElementText()).toInt());
                                    } else {
                                        xml.skipCurrentElement();
                                    }
                                }
                                project->tags()->push_back(tag);
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else if (xml.name() == "preferences") {
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "renderSectionMode") {
                                pr->renderSectionMode() = xml.attributes().value("mode").toString();
                                if (projVersionMajor == 2 && projVersionMinor < 4) {
                                    if (pr->renderSectionMode() == "time") {
                                        pr->renderSectionMode() = "expr";
                                    }
                                }
                                while (xml.readNextStartElement()) {
                                    if (xml.name() == "renderStartTag") {
                                        pr->renderStartTag() = xml.attributes().value("label").toString();
                                        xml.skipCurrentElement();
                                    } else if (xml.name() == "renderEndTag") {
                                        pr->renderEndTag() = xml.attributes().value("label").toString();
                                        xml.skipCurrentElement();
                                    } else if (xml.name() == "renderStartTime") {
                                        pr->renderStartTime() = xml.attributes().value("time").toString();
                                        xml.skipCurrentElement();
                                    } else if (xml.name() == "renderEndTime") {
                                        pr->renderEndTime() = xml.attributes().value("time").toString();
                                        xml.skipCurrentElement();
                                    } else {
                                        xml.skipCurrentElement();
                                    }
                                }

                            } else if (xml.name() == "renderFrameSize") {
                                pr->renderFrameSize() = (FrameSize) xml.attributes().value("size").toString().toInt();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "renderInterpolationType") {
                                pr->renderInterpolationType() = (InterpolationType) xml.attributes().value("type").toString().toInt();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "renderMotionblurType") {
                                pr->renderMotionblurType() = (MotionblurType) xml.attributes().value("type").toString().toInt();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "renderFPS") {
                                if (projVersionMajor == 2 && projVersionMinor <= 2) {
                                    pr->renderFPS() = xml.attributes().value("fps").toString().toFloat();
                                } else {
                                    pr->renderFPS() = xml.attributes().value("fps").toString();
                                }
                                xml.skipCurrentElement();

                            } else if (xml.name() == "renderSlowmoSamples") {
                                project->motionBlur()->setSlowmoSamples(xml.attributes().value("number").toString().toInt());
                                xml.skipCurrentElement();
                            } else if (xml.name() == "renderMaxSamples") {
                                project->motionBlur()->setMaxSamples(xml.attributes().value("number").toString().toInt());
                                xml.skipCurrentElement();

                            } else if (xml.name() == "renderTarget") {
                                pr->renderTarget() = xml.attributes().value("target").toString();
                                xml.skipCurrentElement();

                            } else if (xml.name() == "imagesOutputDir") {
                                pr->imagesOutputDir() = xml.attributes().value("dir").toString();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "imagesFilenamePattern") {
                                pr->imagesFilenamePattern() = xml.attributes().value("pattern").toString();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "videoFilename") {
                                QString filename = xml.attributes().value("file").toString();
                                if (!filename.isEmpty()) {
                                    pr->videoFilename() = filename;
                                }
                                xml.skipCurrentElement();
                            } else if (xml.name() == "videoCodec") {
                                pr->videoCodec() = xml.attributes().value("codec").toString();
                                xml.skipCurrentElement();

                            } else if (xml.name() == "flowV3dLambda") {
                                pr->flowV3DLambda() = xml.attributes().value("lambda").toString().toFloat();
                                xml.skipCurrentElement();

                            } else if (xml.name() == "prevTagAxis") {
                                pr->lastSelectedTagAxis() = (TagAxis)xml.attributes().value("axis").toString().toInt();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "viewport_t0") {
                                pr->viewport_t0().rx() = xml.attributes().value("x").toString().toFloat();
                                pr->viewport_t0().ry() = xml.attributes().value("y").toString().toFloat();
                                xml.skipCurrentElement();
                            } else if (xml.name() == "viewport_secRes") {
                                pr->viewport_secRes().rx() = xml.attributes().value("x").toString().toFloat();
                                pr->viewport_secRes().ry() = xml.attributes().value("y").toString().toFloat();
                                xml.skipCurrentElement();

                            } else if (xml.name() == "canvas_xAxisFPS") {
                                pr->canvas_xAxisFPS() = xml.attributes().value("fps").toString();
                                xml.skipCurrentElement();
                            }


                            else {
                                xml.skipCurrentElement();
                            }
                        }
                    } else {
                        qDebug() << "Unknown element: " << xml.name();
                        xml.skipCurrentElement();
                    }
                }
                xml.readNextStartElement();
                if (xml.name().length() > 0) {
                    qDebug() << "Did not read the whole project file! Stopped at: " << xml.name();
                }
                Q_ASSERT(xml.name().length() == 0);


                file.close();


                // Handle new project versions
                if (projVersionMajor > SLOWMOPROJECT_VERSION_MAJOR && projVersionMajor) {
                    throw Error_sV(QString("This file has been created with slowmoVideo %1.%2.%3 which uses a newer "
                                           "project file version (%4.%5; supported: %6.%7). File cannot be loaded "
                                           "(or only partially). Please upgrade to a newer version of slowmoVideo.")
                                   .arg(version_major).arg(version_minor).arg(version_micro)
                                   .arg(projVersionMajor).arg(projVersionMinor)
                                   .arg(SLOWMOPROJECT_VERSION_MAJOR).arg(SLOWMOPROJECT_VERSION_MINOR));
                } else if (projVersionMinor > SLOWMOPROJECT_VERSION_MINOR) {
                    QString warningMsg = QString("This file has been created with a slightly newer version of slowmoVideo "
                                                 "(version %1.%2.%3) which uses project file version %4.%5 (supported: %6.%7). "
                                                 "When saving this project, some added properties will be lost.")
                                         .arg(version_major).arg(version_minor).arg(version_micro)
                                         .arg(projVersionMajor).arg(projVersionMinor)
                                         .arg(SLOWMOPROJECT_VERSION_MAJOR).arg(SLOWMOPROJECT_VERSION_MINOR);
                    qDebug() << warningMsg;
                    if (warning != NULL) {
                        *warning = warningMsg;
                    }
                }

                return project;

            }
        }
        file.close();
    }
    return NULL;
}
