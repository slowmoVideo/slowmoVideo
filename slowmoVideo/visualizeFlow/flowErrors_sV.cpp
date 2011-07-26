#include "flowErrors_sV.h"

void FlowErrors_sV::difference(FlowField_sV &left, FlowField_sV &right, FlowField_sV &out)
{
    float dx, dy;
    for (int y = 0; y < left.height(); y++) {
        for (int x = 0; x < left.width(); x++) {
            dx = left.x(x,y);
            dy = left.y(x,y);
            if (x+dx >= 0 && y+dy >= 0
                    && x+dx <= left.width()-1 && y+dy <= left.height()-1) {
                dx += right.x(x+dx, y+dy);
                dy += right.y(x+dx, y+dy);
            }
            out.setX(x,y, dx);
            out.setY(x,y, dy);
        }
    }
}
