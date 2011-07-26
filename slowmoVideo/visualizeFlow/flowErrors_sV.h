#ifndef FLOWERRORS_SV_H
#define FLOWERRORS_SV_H

#include "lib/flowField_sV.h"

class FlowErrors_sV
{
public:
    static void difference(FlowField_sV &left, FlowField_sV &right, FlowField_sV &out);
};

#endif // FLOWERRORS_SV_H
