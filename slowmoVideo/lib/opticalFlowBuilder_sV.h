#ifndef OPTICALFLOWBUILDER_SV_H
#define OPTICALFLOWBUILDER_SV_H

#include <QFile>

class OpticalFlowBuilder_sV {

public:
    virtual ~OpticalFlowBuilder_sV() {}

    enum Direction { Direction_Forward, Direction_Backward };

    virtual void buildFlow(const QFile& left, const QFile& right, const QFile& output, Direction direction) const = 0;
};

#endif // OPTICALFLOWBUILDER_SV_H
