#ifndef FLOW_SV_H
#define FLOW_SV_H

#include "project_sV.h"
#include "../lib/defs_sV.h"

#include <QtCore>
#include <QObject>

class Flow_sV : public QObject
{
    Q_OBJECT
public:
    Flow_sV();
    Flow_sV(const Flow_sV &other);
    virtual ~Flow_sV() {}
    Flow_sV& operator=(const Flow_sV &other);

    // See http://www.parashift.com/c++-faq-lite/pointers-to-members.html#faq-33.5
    typedef const QString (Project_sV::*ProjectFrameMemFn)(int nr) const;
    typedef const QString (Project_sV::*ProjectFlowMemFn)(int nr, FlowDirection direction) const;

    void buildFlow(Project_sV *project, ProjectFrameMemFn frameNames, ProjectFlowMemFn outName,
                          FlowDirection direction);

signals:
    void signalFlowProgressUpdated(int progress);
    void signalFlowFrame(const QString& name);
    void signalFlowFinished();
};

#endif // FLOW_SV_H
