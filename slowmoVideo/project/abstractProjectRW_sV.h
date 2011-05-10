#ifndef ABSTRACTPROJECTRW_SV_H
#define ABSTRACTPROJECTRW_SV_H


class QString;
class Project_sV;
class AbstractProjectRW_sV
{
public:
    AbstractProjectRW_sV() {}
    virtual Project_sV* loadProject(QString filename) const;
    virtual int saveProject(const Project_sV *project, QString filename) const;
};

#endif // ABSTRACTPROJECTRW_SV_H
