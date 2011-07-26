#ifndef FLOWTOOLS_SV_H
#define FLOWTOOLS_SV_H

#include "flowField_sV.h"

class FlowTools_sV
{
public:
    static void difference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out);
    static void signedDifference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out);
};

#endif // FLOWTOOLS_SV_H
