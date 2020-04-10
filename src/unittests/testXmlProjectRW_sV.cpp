#include "testXmlProjectRW_sV.h"
#include "../project/xmlProjectRW_sV.h"
#include "../project/nodeList_sV.h"

void TestXmlProjectRW_sV::initTestCase()
{
    int i = 0;
    app = new QCoreApplication(i, NULL);
}
void TestXmlProjectRW_sV::cleanupTestCase()
{
    delete app;
}

void TestXmlProjectRW_sV::init()
{
    m_project = new Project_sV(QDir::tempPath());

    m_project->nodes()->add(Node_sV(0,0));
    m_project->nodes()->add(Node_sV(1,1));
    m_project->nodes()->setCurveType(.5, CurveType_Bezier);
    (*m_project->nodes())[0].setRightNodeHandle(.5, 2);
    (*m_project->nodes())[1].setLeftNodeHandle(-.4, -2);
}

void TestXmlProjectRW_sV::slotTestHandles()
{
    XmlProjectRW_sV rw;
    const QString filename = QString("%1/xprTest.sVproj").arg(QDir::tempPath());
    rw.saveProject(m_project, filename);
    Project_sV *loaded = rw.loadProject(filename);
    qDebug() << "Node handles: "
             << toString(loaded->nodes()->at(0).rightNodeHandle()) << "/"
             << toString(m_project->nodes()->at(0).rightNodeHandle());
    qDebug() << "Node handles: "
             << toString(loaded->nodes()->at(1).leftNodeHandle()) << "/"
             << toString(m_project->nodes()->at(1).leftNodeHandle());
    QVERIFY(loaded->nodes()->at(0).rightNodeHandle() == m_project->nodes()->at(0).rightNodeHandle());
    QVERIFY(loaded->nodes()->at(1).leftNodeHandle() == m_project->nodes()->at(1).leftNodeHandle());
    delete loaded;
}

void TestXmlProjectRW_sV::cleanup()
{
    delete m_project;
}
