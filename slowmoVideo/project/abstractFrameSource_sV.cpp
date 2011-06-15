#include "abstractFrameSource_sV.h"

AbstractFrameSource_sV::AbstractFrameSource_sV(const Project_sV *project) :
    m_project(project)
{
}

AbstractFrameSource_sV::~AbstractFrameSource_sV()
{

}


float AbstractFrameSource_sV::fps() const throw(Div0Exception)
{
    int num = frameRateNum();
    int den = frameRateDen();
    if (den == 0) {
        throw Div0Exception();
    }
    return float(num)/float(den);
}
