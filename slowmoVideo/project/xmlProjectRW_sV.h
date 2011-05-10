#ifndef XMLPROJECTRW_SV_H
#define XMLPROJECTRW_SV_H

#include "abstractProjectRW_sV.h"

#include <QtXml>

class Node_sV;
class XmlProjectRW_sV// : public AbstractProjectRW_sV
{
public:
    explicit XmlProjectRW_sV();
    Project_sV* loadProject(QString filename) const;
    int saveProject(const Project_sV *project, QString filename) const;

private:
    static const QDomElement nodeToDom(QDomDocument *doc, const Node_sV *node);
};

#endif // XMLPROJECTRW_SV_H
