#ifndef PROJECTPREFERENCES_SV_H
#define PROJECTPREFERENCES_SV_H

#include "../lib/defs_sV.hpp"
class ProjectPreferences_sV
{
public:
    ProjectPreferences_sV();

    /**
      \param tagAxis If set to a value >= 0, remember the value.
      \return the previously selected tag axis.
      */
    TagAxis lastSelectedTagAxis(int tagAxis = -1);

private:
    TagAxis m_tagAxis;

};

#endif // PROJECTPREFERENCES_SV_H
