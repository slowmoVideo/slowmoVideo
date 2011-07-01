#include "projectPreferences_sV.h"

ProjectPreferences_sV::ProjectPreferences_sV() :
    m_tagAxis(TagAxis_Source)
{
}

TagAxis ProjectPreferences_sV::lastSelectedTagAxis(int tagAxis)
{
    TagAxis prev = m_tagAxis;
    if (tagAxis >= 0) {
        m_tagAxis = (TagAxis) tagAxis;
    }
    return prev;
}
