/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef FLOW_SV_H
#define FLOW_SV_H

class Project_sV;
#include "project_sV.h"
#include "../lib/defs_sV.h"
#include "../lib/opticalFlowBuilder_sV.h"

#include <QtCore>
#include <QObject>

class Flow_sV : public QObject
{
    Q_OBJECT

public:
    Flow_sV();
    ~Flow_sV();

    // See http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.5
    typedef const QString (Project_sV::*ProjectFrameMemFn)(int nr) const;
    typedef const QString (Project_sV::*ProjectFlowMemFn)(int nr, FlowDirection direction) const;

    void buildFlow(Project_sV *project, ProjectFrameMemFn frameNames, ProjectFlowMemFn outName,
                          FlowDirection direction);
    void buildFlowImage(const QString& leftFrame, const QString& rightFrame, const QString& outFrame, FlowDirection direction);

signals:
    void signalFlowProgressUpdated(int progress);
    void signalFlowFrame(const QString& name);
    void signalFlowFinished();
    void signalFlowAborted();

public slots:
    void slotAbort();

private:
    OpticalFlowBuilder_sV *m_flowBuilder;

    bool m_abort;
    bool m_aborted;

};

#endif // FLOW_SV_H
