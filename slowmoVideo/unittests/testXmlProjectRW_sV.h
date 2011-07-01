#ifndef TESTXMLPROJECTRW_SV_H
#define TESTXMLPROJECTRW_SV_H

#include "../project/project_sV.h"

#include <QObject>
#include <QtTest/QtTest>


class TestXmlProjectRW_sV : public QObject
{
    Q_OBJECT

private slots:
    void slotTestHandles();
    void init();
    void cleanup();

private:
    Project_sV *m_project;

};

#endif // TESTXMLPROJECTRW_SV_H
